#include "MIPI_DSI_LP_Analyzer.h"
#include "MIPI_DSI_LP_AnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <fstream>
#include <string>
#include <iostream>

//#define DEBUG

MIPI_DSI_LP_Analyzer::MIPI_DSI_LP_Analyzer()
:	Analyzer2(),  
	mSettings( new MIPI_DSI_LP_AnalyzerSettings() ),
	mSimulationInitialized(false)
{
	SetAnalyzerSettings( mSettings.get() );
	pFile = NULL;
}

MIPI_DSI_LP_Analyzer::~MIPI_DSI_LP_Analyzer()
{
	KillThread();
}

void MIPI_DSI_LP_Analyzer::SetupResults()
{
	mResults.reset( new MIPI_DSI_LP_AnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mPosChannel );
}

#ifdef DEBUG
	#define DEBUG_PRINTF(...)		{fprintf(pFile, __VA_ARGS__);fprintf(pFile, "\n");}
#else
	#define DEBUG_PRINTF(...)		{}
#endif

void MIPI_DSI_LP_Analyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();
	sampleStart = 0;
	pulseLength = 0;

	mDataP = GetAnalyzerChannelData( mSettings->mPosChannel );
	mDataN = GetAnalyzerChannelData( mSettings->mNegChannel );

#ifdef DEBUG
	pFile = fopen("log.txt", "w");
#endif

	DEBUG_PRINTF("Log started.");

	for ( ; ; )
	{
		DEBUG_PRINTF("Loop.");
		CheckIfThreadShouldExit();

		if (!GetStart()) {
			mResults->CommitResults();
			continue;
		}

		if (GetBitstream() >= 8) {
			GetData();
		}
	}

#ifdef DEBUG
	fclose(pFile);
#endif
}

bool MIPI_DSI_LP_Analyzer::GetStart()
{
	/* D+ and D- should be at the same sample here. */
	{
		/* If D+ is behind D-. */
		if (mDataP->GetSampleNumber() < mDataN->GetSampleNumber()) {
			/* Advance D+ to D-. */
			mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());
			return false;
		}
		/* If D- is behind D+. */
		if (mDataN->GetSampleNumber() < mDataP->GetSampleNumber()) {
			/* Advance D- to D+. */
			mDataN->AdvanceToAbsPosition(mDataP->GetSampleNumber());
			return false;
		}
	}

	/* Both D+ and D- must be high for start condition. */
	{
		/* Check D+. */
		if (mDataP->GetBitState() != BIT_HIGH) {
			/* Advance D+. */
			mDataP->AdvanceToNextEdge();
			return false;
		}
		/* Check D-. */
		if (mDataN->GetBitState() != BIT_HIGH) {
			/* Advance D-. */
			mDataN->AdvanceToNextEdge();
			return false;
		}
	}

	/* D+ and D- are both high now. For a start condition, D- should go low first. */
	/* Check if D+ goes low first instead. */
	if (mDataP->GetSampleOfNextEdge() <= mDataN->GetSampleOfNextEdge()) {
		/* Advance D+. */
		mDataP->AdvanceToNextEdge();
		return false;
	}

	/* D- goes low first, advance to falling edge. */
	mDataN->AdvanceToNextEdge();
	/* Advance D+ to D-. */
	mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());

	/* D- is low, D+ is high, next transition should be D+ going low. */
	/* Check if next transition is on D- instead. */
	if (mDataN->GetSampleOfNextEdge() <= mDataP->GetSampleOfNextEdge()) {
		/* Advance D-. */
		mDataN->AdvanceToNextEdge();
		return false;
	}

	/* Advance D+ to falling edge. */
	mDataP->AdvanceToNextEdge();
	/* Advance D- to D+. */
	mDataN->AdvanceToAbsPosition(mDataP->GetSampleNumber());

	/* D+ and D- are both low now. Next transition should be on D-. */
	/* Check if next transition is on D+ instead. */
	if (mDataP->GetSampleOfNextEdge() <= mDataN->GetSampleOfNextEdge()) {
		/* Advance D+. */
		mDataP->AdvanceToNextEdge();
		return false;
	}

	/* Remember this position as possible start. */
	sampleStart = mDataP->GetSampleNumber();
	DEBUG_PRINTF("Possible start detected. sampleStart = %lld", sampleStart);

	/* Calculate length from start to pulse. */
	U64 startToPulse = mDataN->GetSampleOfNextEdge() - sampleStart;

	/* Go to D- rising edge. */
	mDataN->AdvanceToNextEdge();

	/* Calculate bitrate lenght using D- pulse. */
	pulseLength = mDataN->GetSampleOfNextEdge() - mDataN->GetSampleNumber();

	DEBUG_PRINTF("pulseLength = %lld", pulseLength);

	/* Go to D- falling edge. */
	mDataN->AdvanceToNextEdge();

	/* Check if edge timings are outside boundary. */
	if (startToPulse > (pulseLength * 5)) {
		DEBUG_PRINTF("Error: D- pulse timing outside boundary.");
		mResults->AddMarker(sampleStart, AnalyzerResults::ErrorX, mSettings->mNegChannel);
		return false;
	}

	/* Check if D+ was low during D- pulse. */
	if (mDataP->WouldAdvancingToAbsPositionCauseTransition(mDataN->GetSampleNumber())) {
		DEBUG_PRINTF("Error: D+ was not low during D- pulse.");
		/* D+ was not low, that's an error. */
		mResults->AddMarker(mDataN->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mNegChannel);
		/* Advance D+ to D-. */
		mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());
		return false;
	}

	/* Timings are ok: this is a start. */
	DEBUG_PRINTF("Start is OK.");
	mResults->AddMarker(sampleStart, AnalyzerResults::Start, mSettings->mPosChannel);

	/* Advance D+ to D-. */
	mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());
	/* Mark D- falling edge. */
	mResults->AddMarker(mDataN->GetSampleNumber(), AnalyzerResults::DownArrow, mSettings->mNegChannel);

	/* D+ and D- are at the falling edge sample of D- pulse here. */
	return true;
}

U64 MIPI_DSI_LP_Analyzer::GetBitstream()
{
	U64 bitCounter = 0xFFFF * 8; /* Max number of bits allowed. */

	data.clear();

	/* Get bitstream. */
	while (--bitCounter)
	{
		DEBUG_PRINTF("GetBitstream() loop. bit_counter = %lld, Dp = %lld , Dm = %lld", bitCounter, mDataP->GetSampleNumber(), mDataN->GetSampleNumber());

		/* D+ and D- should be at the same sample here. */
		{
			/* If D+ is behind D-. */
			if (mDataP->GetSampleNumber() < mDataN->GetSampleNumber()) {
				/* Advance D+ to D-. */
				mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());
			}
			/* If D- is behind D+. */
			if (mDataN->GetSampleNumber() < mDataP->GetSampleNumber()) {
				/* Advance D- to D+. */
				mDataN->AdvanceToAbsPosition(mDataP->GetSampleNumber());
			}
		}

		/* Both D+ and D- should be low here. */
		{
			/* Check if D+ high. */
			if (mDataP->GetBitState() == BIT_HIGH) {
				/* Add error marker. */
				mResults->AddMarker(mDataP->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mPosChannel);
				break;
			}
			/* Check if D- is high. */
			if (mDataN->GetBitState() == BIT_HIGH) {
				/* Add error marker. */
				mResults->AddMarker(mDataN->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mNegChannel);
				break;
			}
		}

		/* Check on which data line we've got next edge. */
		if (mDataP->GetSampleOfNextEdge() <= mDataN->GetSampleOfNextEdge()) {
			/* Check if next edge is too far. */
			if ((mDataP->GetSampleOfNextEdge() - mDataP->GetSampleNumber()) >= (pulseLength * 5)) {
				DEBUG_PRINTF("Error: D+ too far.");
				/* Go to rising edge. */
				mDataP->AdvanceToNextEdge();
				/* Add error marker. */
				mResults->AddMarker(mDataP->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mPosChannel);
				/* Advance D- to D+. */
				mDataN->AdvanceToAbsPosition(mDataP->GetSampleNumber());
				/* Exit bitsteam. */
				break;
			}

			/* Go to rising edge. */
			mDataP->AdvanceToNextEdge();
			/* Advance D- to D+. */
			mDataN->AdvanceToAbsPosition(mDataP->GetSampleNumber());

			/* If this is a bit on D+, there won't be any transitions on D- until D+ falling edge. */
			/* Check if there's a transition on D- until D+ falling edge making it a stop condition. */
			/* Check if there are no more transitions on D+ (so there won't be a falling edge) but there is a transition on D- making it a stop condition. */
			if ((mDataN->GetSampleOfNextEdge() <= mDataP->GetSampleOfNextEdge()) || (!mDataP->DoMoreTransitionsExistInCurrentData() && mDataN->DoMoreTransitionsExistInCurrentData())) {
				/* Advance D-. */
				mDataN->AdvanceToNextEdge();
				/* Both D+ and D- are high now, this is stop. */
				DEBUG_PRINTF("Stop condition @ %lld / data.size() = %lli", mDataP->GetSampleNumber(), data.size());
				mResults->AddMarker(mDataP->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mPosChannel);
				/* Exit bitsteam. */
				DEBUG_PRINTF("Exit bitstream.");
				break;
			}

			/* Bit's on D+. */
			Bit bit;
			DEBUG_PRINTF("D+ bit.");

			/* Remember bit stats. */
			bit.sampleBegin = mDataP->GetSampleNumber();
			bit.sampleEnd = mDataP->GetSampleOfNextEdge();

			/* Check if next edge is too far. */
			if ((bit.sampleEnd - bit.sampleBegin) >= (pulseLength * 5)) {
				DEBUG_PRINTF("Next edge too far.");
				/* Mark an error at sample begin. */
				mResults->AddMarker(bit.sampleBegin, AnalyzerResults::ErrorX, mSettings->mPosChannel);
				/* Advance D+ over this long pulse (to bit.sampleEnd). */
				mDataP->AdvanceToNextEdge();
				/* Advance D-. */
				mDataN->AdvanceToAbsPosition(mDataP->GetSampleNumber());
				/* Exit bitsteam. */
				break;
			}

			bit.value = BIT_HIGH;
			/* Mark the bit in the middle. */
			mResults->AddMarker((bit.sampleBegin >> 1) + (bit.sampleEnd >> 1), AnalyzerResults::One, mSettings->mPosChannel);

			/* Go to falling edge (bit.sampleEnd). */
			mDataP->AdvanceToNextEdge();
			/* Save the bit. */
			data.push_back(bit);
			/* Advance D- to bit's end. */
			mDataN->AdvanceToAbsPosition(mDataP->GetSampleNumber());
		}
		else {
			/* Check if next edge is too far. */
			if ((mDataN->GetSampleOfNextEdge() - mDataN->GetSampleNumber()) >= (pulseLength * 5)) {
				DEBUG_PRINTF("Error: D- too far.");
				/* Go to rising edge. */
				mDataN->AdvanceToNextEdge();
				/* Add error marker. */
				mResults->AddMarker(mDataN->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mNegChannel);
				/* Advance D+ to D-. */
				mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());
				/* Exit bitsteam. */
				break;
			}

			/* Go to rising edge. */
			mDataN->AdvanceToNextEdge();
			/* Advance D+ to D-. */
			mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());

			/* If this is a bit on D-, there won't be any transitions on D+ until D- falling edge. */
			/* Check if there's a transition on D+ until D- falling edge making it a (failed) stop condition. */
			/* Check if there are no more transitions on D- (so there won't be a falling edge) but there is a transition on D+ making it a (failed) stop condition. */
			if ((mDataP->GetSampleOfNextEdge() <= mDataN->GetSampleOfNextEdge()) || (!mDataN->DoMoreTransitionsExistInCurrentData() && mDataP->DoMoreTransitionsExistInCurrentData())) {
				/* Advance D+. */
				mDataP->AdvanceToNextEdge();
				/* Both D+ and D- are high now, this is failed stop (as stop occurs with D+ going high first). */
				DEBUG_PRINTF("Failed Stop condition on D- @ %lld / data.size() = %lli", mDataP->GetSampleNumber(), data.size());
				mResults->AddMarker(mDataN->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mNegChannel);
				/* Exit bitsteam. */
				DEBUG_PRINTF("Exit bitstream.");
				break;
			}

			/* Bit's on D-. */
			Bit bit;
			DEBUG_PRINTF("D- bit.");

			/* Remember bit stats. */
			bit.sampleBegin = mDataN->GetSampleNumber();
			bit.sampleEnd = mDataN->GetSampleOfNextEdge();

			/* Check if next edge is too far. */
			if ((bit.sampleEnd - bit.sampleBegin) >= (pulseLength * 5)) {
				DEBUG_PRINTF("Next edge too far.");
				/* Mark an error at sample begin. */
				mResults->AddMarker(bit.sampleBegin, AnalyzerResults::ErrorX, mSettings->mNegChannel);
				/* Advance D- over this long pulse (to bit.sampleEnd). */
				mDataN->AdvanceToNextEdge();
				/* Advance D+. */
				mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());
				/* Exit bitsteam. */
				break;
			}

			bit.value = BIT_LOW;
			/* Mark the bit in the middle. */
			mResults->AddMarker((bit.sampleBegin >> 1) + (bit.sampleEnd >> 1), AnalyzerResults::Zero, mSettings->mNegChannel);

			/* Go to falling edge (bit.sampleEnd). */
			mDataN->AdvanceToNextEdge();
			/* Save the bit. */
			data.push_back(bit);
			/* Advance D+ to bit's end. */
			mDataP->AdvanceToAbsPosition(mDataN->GetSampleNumber());
		}
	}

	mResults->CommitResults();
	return data.size();
}

U64 MIPI_DSI_LP_Analyzer::GetData(void)
{
	Frame frame;
	U64 byteIndex, byteCount;
	U64 packetType;

	/* Initialize frame's fields. */
	frame.mStartingSampleInclusive = 0;
	frame.mEndingSampleInclusive = 0;
	frame.mData1 = 0;
	frame.mData2 = 0;
	frame.mFlags = 0;
	frame.mType = 0;

	/* Number of bytes in the stream. */
	byteCount = data.size() / 8U;
	byteIndex = 0U;

	if (byteCount == 4U) {
		/* Short packet. */
		packetType = MIPI_DSI_PACKET_SHORT;
	} else
	if (byteCount >= 6U) {
		/* Long packet */
		packetType = MIPI_DSI_PACKET_LONG;
	} else
	{
		/* Unrecognized packet. */
		packetType = MIPI_DSI_PACKET_UNRECOGNIZED;
	}

	/* Extract bitstream by 8 bits. */
	while (data.size() >= 8)
	{
		DataBuilder byteBuilder;
		U64 byte = 0;

		/* Mark first and last samples of a frame. */
		frame.mStartingSampleInclusive = data.front().sampleBegin;
		frame.mEndingSampleInclusive = data.at(7).sampleEnd;

		/* Reset byteBuilder onto byte. */
		byteBuilder.Reset(&byte, AnalyzerEnums::LsbFirst, 8);
		/* Loop through 8 bits of a data. */
		for (int i = 0; i < 8; i++) byteBuilder.AddBit(data.at(i).value);
		/* Remove those 8 bits. */
		data.erase(data.begin(), data.begin() + 8);

		/* Fill frame with data. */
		frame.mData1 = byte;
		frame.mData2 = ((byteCount & UINT32_MAX) << 32U) | (byteIndex & UINT32_MAX);
		frame.mType = (U8)packetType;

		mResults->AddFrame(frame);
		byteIndex++;
	}

	U64 bitsRemaining = data.size(); /* Ideally all bits have been processed. */
	data.clear();
	mResults->CommitResults();

	return bitsRemaining;
}

bool MIPI_DSI_LP_Analyzer::NeedsRerun()
{
	return false;
}

U32 MIPI_DSI_LP_Analyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	if (mSimulationInitialized == false)
	{
		mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
		mSimulationInitialized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index, device_sample_rate, simulation_channels);
}

U32 MIPI_DSI_LP_Analyzer::GetMinimumSampleRateHz()
{
	return 1000000U; /* 1MHz */
}

const char* MIPI_DSI_LP_Analyzer::GetAnalyzerName() const
{
	return "MIPI DSI LP mode";
}

const char* GetAnalyzerName()
{
	return "MIPI DSI LP mode";
}

Analyzer* CreateAnalyzer()
{
	return new MIPI_DSI_LP_Analyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
