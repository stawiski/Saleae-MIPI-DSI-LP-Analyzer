#ifndef MIPI_DSI_LP__ANALYZER_RESULTS
#define MIPI_DSI_LP__ANALYZER_RESULTS

#include <AnalyzerResults.h>

class MIPI_DSI_LP_Analyzer;
class MIPI_DSI_LP_AnalyzerSettings;

class MIPI_DSI_LP_AnalyzerResults : public AnalyzerResults
{
public:
	MIPI_DSI_LP_AnalyzerResults( MIPI_DSI_LP_Analyzer* analyzer, MIPI_DSI_LP_AnalyzerSettings* settings );
	virtual ~MIPI_DSI_LP_AnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

protected: //functions

protected:  //vars
	MIPI_DSI_LP_AnalyzerSettings* mSettings;
	MIPI_DSI_LP_Analyzer* mAnalyzer;
	uint32_t DSI_packetsCount;
};

#endif //MIPI_DSI_LP__ANALYZER_RESULTS
