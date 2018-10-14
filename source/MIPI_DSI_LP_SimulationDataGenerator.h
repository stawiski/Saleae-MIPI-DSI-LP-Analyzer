#ifndef MIPI_DSI_LP__SIMULATION_DATA_GENERATOR
#define MIPI_DSI_LP__SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>
#include <SimulationChannelDescriptor.h>
#include <string>
#include <stdlib.h>
#include "MIPI_DSI_LP_AnalyzerSettings.h"

class MIPI_DSI_LP_SimulationDataGenerator
{
public:
	MIPI_DSI_LP_SimulationDataGenerator();
	~MIPI_DSI_LP_SimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, MIPI_DSI_LP_AnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );

protected:
	MIPI_DSI_LP_AnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:

	SimulationChannelDescriptorGroup mSimulationChannels;
	SimulationChannelDescriptor* mSimulationChannelDataP;
	SimulationChannelDescriptor* mSimulationChannelDataN;

};
#endif //MIPI_DSI_LP__SIMULATION_DATA_GENERATOR