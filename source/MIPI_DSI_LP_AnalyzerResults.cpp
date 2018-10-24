#include "MIPI_DSI_LP_AnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "MIPI_DSI_LP_Analyzer.h"
#include "MIPI_DSI_LP_AnalyzerSettings.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>

struct MIPI_DSI_Packet {
	uint8_t DataType;
	int32_t parametersCount;
	std::string description;
};

struct MIPI_DSI_Packet DSI_packets[] =
{
	{0x01, -1, "Sync Event, V Sync Start"},
	{0x11, -1, "Sync Event, V Sync End"},
	{0x21, -1, "Sync Event, H Sync Start"},
	{0x31, -1, "Sync Event, H Sync End"},
	{0x07, -1, "Compression Mode Command"},
	{0x08, -1, "End of Transmission packet(EoTp)"},
	{0x02, -1, "Color Mode(CM) Off Command"},
	{0x12, -1, "Color Mode(CM) On Command"},
	{0x22, -1, "Shut Down Peripheral Command"},
	{0x32, -1, "Turn On Peripheral Command"},
	{0x03, 0, "Generic Short WRITE"},
	{0x13, 1, "Generic Short WRITE"},
	{0x23, 2, "Generic Short WRITE"},
	{0x04, 0, "Generic READ"},
	{0x14, 1, "Generic READ"},
	{0x24, 2, "Generic READ"},
	{0x05, 0, "DCS Short WRITE"},
	{0x15, 1, "DCS Short WRITE"},
	{0x06, 0, "DCS READ"},
	{0x16, -1, "Execute Queue"},
	{0x37, -1, "Set Maximum Return Packet Size"},
	{0x09, -1, "Null Packet"},
	{0x19, -1, "Blanking Packet"},
	{0x29, -1, "Generic Long Write"},
	{0x39, -1, "DCS Long Write / write_LUT Command Packet"},
	{0x0A, -1, "Picture Parameter Set"},
	{0x0B, -1, "Compressed Pixel Stream"},
	{0x0C, -1, "Loosely Packed Pixel Stream, 20 - bit YCbCr, 4:2 : 2 Format"},
	{0x1C, -1, "Packed Pixel Stream, 24 - bit YCbCr, 4 : 2 : 2 Format"},
	{0x2C, -1, "Packed Pixel Stream, 16 - bit YCbCr, 4 : 2 : 2 Format"},
	{0x0D, -1, "Packed Pixel Stream, 30 - bit RGB, 10 - 10 - 10 Format"},
	{0x1D, -1, "Packed Pixel Stream, 36 - bit RGB, 12 - 12 - 12 Format"},
	{0x3D, -1, "Packed Pixel Stream, 12 - bit YCbCr, 4 : 2 : 0 Format"},
	{0x0E, -1, "Packed Pixel Stream, 16 - bit RGB, 5 - 6 - 5 Format"},
	{0x1E, -1, "Packed Pixel Stream, 18 - bit RGB, 6 - 6 - 6 Format"},
	{0x2E, -1, "Loosely Packed Pixel Stream, 18 - bit RGB, 6 - 6 - 6 Format"},
	{0x3E, -1, "Packed Pixel Stream, 24 - bit RGB, 8 - 8 - 8 Format"}
};

MIPI_DSI_LP_AnalyzerResults::MIPI_DSI_LP_AnalyzerResults( MIPI_DSI_LP_Analyzer* analyzer, MIPI_DSI_LP_AnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
	DSI_packetsCount = sizeof(DSI_packets) / sizeof(DSI_packets[0]);
}

MIPI_DSI_LP_AnalyzerResults::~MIPI_DSI_LP_AnalyzerResults()
{
}

void MIPI_DSI_LP_AnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	char number_str[128];
	static int32_t lastDTindex = -1;

	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

	/* Convert the data byte into a string for generic result string. */
	AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);

	/* Check if it's the first byte of the packet. */
	if ((frame.mData2 & UINT32_MAX) == 0U) {
		char number_str_VC[16];
		char number_str_DT[16];
		std::stringstream ss;

		ss << "DI [" << number_str << "]";
		AddResultString(ss.str().c_str());
		ss.str("");

		/* Virtual Channel Field is DI[7:6]. */
		AnalyzerHelpers::GetNumberString((frame.mData1 >> 6) & 0x3, display_base, 2, number_str_VC, 16);
		/* Data Type Field is DI[5:0]. */
		AnalyzerHelpers::GetNumberString(frame.mData1 & 0x3F, display_base, 6, number_str_DT, 16);

		/* Reset last known DT index. */
		lastDTindex = -1;
		/* Loop through DSI_packets array. */
		for (uint32_t i = 0; i < DSI_packetsCount; i++) {
			/* Check if data type field matches. */
			if (DSI_packets[i].DataType == (frame.mData1 & 0x3F)) {
				/* Remember last known DT index. */
				lastDTindex = i;
				break;
			}
		}

		/* If Data Type is known. */
		if (lastDTindex != -1) {
			/* Show data type and description. */
			ss << "VC [" << number_str_VC << "] " << " DT [" << number_str_DT << "] = " << DSI_packets[lastDTindex].description;
			AddResultString(ss.str().c_str());
			ss.str("");

			/* Show data type, description, and number of parameters (if applicable). */
			ss << "VC [" << number_str_VC << "] " << " DT [" << number_str_DT << "] = " << DSI_packets[lastDTindex].description << ((DSI_packets[lastDTindex].parametersCount >= 0) ? (" (" + std::to_string(DSI_packets[lastDTindex].parametersCount) + " parameters)"):"");
			AddResultString(ss.str().c_str());
			ss.str("");
		} else {
		/* Data Type is unknown. */
			ss << "VC [" << number_str_VC << "] " << "DT [" << number_str_DT << "]";
			AddResultString(ss.str().c_str());
			ss.str("");

			ss << "Virtual Channel [" << number_str_VC << "] " << "Data Type [" << number_str_DT << "]";
			AddResultString(ss.str().c_str());
			ss.str("");
		}
	} else {
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
