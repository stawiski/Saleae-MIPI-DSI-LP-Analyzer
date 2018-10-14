#include "MIPI_DSI_LP_SimulationDataGenerator.h"
#include "MIPI_DSI_LP_AnalyzerSettings.h"

#include <AnalyzerHelpers.h>

MIPI_DSI_LP_SimulationDataGenerator::MIPI_DSI_LP_SimulationDataGenerator()
{
}

MIPI_DSI_LP_SimulationDataGenerator::~MIPI_DSI_LP_SimulationDataGenerator()
{
}

void MIPI_DSI_LP_SimulationDataGenerator::Initialize( U32 simulation_sample_rate, MIPI_DSI_LP_AnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mSimulationChannelDataP = mSimulationChannels.Add(settings->mPosChannel, simulation_sample_rate, BIT_HIGH);
	mSimulationChannelDataN = mSimulationChannels.Add(settings->mNegChannel, simulation_sample_rate, BIT_HIGH);
}

U32 MIPI_DSI_LP_SimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

	while (mSimulationChannelDataP->GetCurrentSampleNumber() < adjusted_largest_sample_requested)
	{
		//CreateSerialByte();
		U32 samples_per_bit = mSimulationSampleRateHz / 1000000U;

		mSimulationChannels.AdvanceAll(samples_per_bit * 10U);
		//mSimulationChannelDataP->Transition();
		//mSimulationChannelDataN->Transition();
		//mSimulationChannels.AdvanceAll(samples_per_bit * 10U);
	}

	*simulation_channels = mSimulationChannels.GetArray();
	return mSimulationChannels.GetCount();
}

