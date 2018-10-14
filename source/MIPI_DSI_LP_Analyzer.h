#ifndef MIPI_DSI_LP__ANALYZER_H
#define MIPI_DSI_LP__ANALYZER_H

#define _CRT_SECURE_NO_WARNINGS /* Disable warning about unsafe fopen(...). */

#include <Analyzer.h>
#include <AnalyzerHelpers.h>
#include "MIPI_DSI_LP_AnalyzerResults.h"
#include "MIPI_DSI_LP_SimulationDataGenerator.h"

/* Define Bit structure. */
struct Bit
{
	U64 sampleBegin, sampleEnd;
	BitState value;
};

class MIPI_DSI_LP_AnalyzerSettings;
class ANALYZER_EXPORT MIPI_DSI_LP_Analyzer : public Analyzer2
{
public:
	MIPI_DSI_LP_Analyzer();
	virtual ~MIPI_DSI_LP_Analyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: class <...> needs to have dll-interface to be used by clients of class

protected: // functions
	bool GetStart(void);
	U64 GetBitstream(void);
	U64 GetData(void);

protected: // vars
	std::auto_ptr< MIPI_DSI_LP_AnalyzerSettings > mSettings;
	std::auto_ptr< MIPI_DSI_LP_AnalyzerResults > mResults;
	AnalyzerChannelData *mDataP, *mDataN;

	MIPI_DSI_LP_SimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitialized;

	// Analyzer vars:
	U32 mSampleRateHz;
	U64 sampleStart;
	U64 pulseLength;
	std::vector<Bit> data;
	// For debug
	FILE *pFile;
#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //MIPI_DSI_LP__ANALYZER_H
