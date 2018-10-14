#include "MIPI_DSI_LP_AnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "MIPI_DSI_LP_Analyzer.h"
#include "MIPI_DSI_LP_AnalyzerSettings.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>

MIPI_DSI_LP_AnalyzerResults::MIPI_DSI_LP_AnalyzerResults( MIPI_DSI_LP_Analyzer* analyzer, MIPI_DSI_LP_AnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

MIPI_DSI_LP_AnalyzerResults::~MIPI_DSI_LP_AnalyzerResults()
{
}

void MIPI_DSI_LP_AnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	char number_str[128];

	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

	AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);

	if (frame.mType == MIPI_DSI_PACKET_SHORT)
	{
		/* Always show at least the data. */
		AddResultString(number_str);

		/* Check if it's the first byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 0U) {
			std::stringstream ss;

			ss << "DI [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's the second byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 1U) {
			std::stringstream ss;

			ss << "DATA0 [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's the third byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 2U) {
			std::stringstream ss;

			ss << "DATA1 [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's the fourth byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 3U) {
			std::stringstream ss;

			ss << "ECC [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		}
	} else
	if (frame.mType == MIPI_DSI_PACKET_LONG)
	{
		/* Always show at least the data. */
		AddResultString(number_str);

		/* Check if it's the first byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 0U) {
			std::stringstream ss;

			ss << "DI [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's the second byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 1U) {
			std::stringstream ss;

			ss << "WC [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		}
		else
		/* Check if it's the third byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 2U) {
			std::stringstream ss;

			ss << "WC [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's the fourth byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == 3U) {
			std::stringstream ss;

			ss << "ECC [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's the second to last byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == (((frame.mData2 >> 32) & UINT32_MAX) - 2U)) {
			std::stringstream ss;

			ss << "CRC [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's the last byte of the packet. */
		if ((frame.mData2 & UINT32_MAX) == (((frame.mData2 >> 32) & UINT32_MAX) - 1U)) {
			std::stringstream ss;

			ss << "CRC [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		} else
		/* Check if it's a data packet (between ECC and CRC). */
		if (((frame.mData2 & UINT32_MAX) > 3U) && (frame.mData2 & UINT32_MAX) < (((frame.mData2 >> 32) & UINT32_MAX) - 2U)) {
			std::stringstream ss;

			ss << "DATA [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
			
			ss << "DATA[" << ((frame.mData2 & UINT32_MAX) - 4U) << "] = [" << number_str << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		}
	} else {
		/* Packet unrecognized. */
		AddResultString(number_str);
	}
}

void MIPI_DSI_LP_AnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

		file_stream << time_str << "," << number_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void MIPI_DSI_LP_AnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	Frame frame = GetFrame( frame_index );
	ClearTabularText();

	char number_str[128];
	AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	AddTabularText( number_str );
#endif
}

void MIPI_DSI_LP_AnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	// Not supported
}

void MIPI_DSI_LP_AnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	// Not supported
}
