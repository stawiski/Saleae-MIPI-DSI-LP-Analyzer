#include "MIPI_DSI_LP_AnalyzerSettings.h"
#include <AnalyzerHelpers.h>

MIPI_DSI_LP_AnalyzerSettings::MIPI_DSI_LP_AnalyzerSettings()
:	mPosChannel(UNDEFINED_CHANNEL), mNegChannel(UNDEFINED_CHANNEL)
{
	mSettingChannelP.reset(new AnalyzerSettingInterfaceChannel());
	mSettingChannelP->SetTitleAndTooltip( "DATA+", "" );
	mSettingChannelP->SetChannel(mPosChannel);

	mSettingChannelN.reset(new AnalyzerSettingInterfaceChannel());
	mSettingChannelN->SetTitleAndTooltip("DATA-", "");
	mSettingChannelN->SetChannel(mNegChannel);

	AddInterface(mSettingChannelP.get());
	AddInterface(mSettingChannelN.get());

	ClearChannels();
	AddChannel(mPosChannel, "D+", false);
	AddChannel(mNegChannel, "D-", false);
}

MIPI_DSI_LP_AnalyzerSettings::~MIPI_DSI_LP_AnalyzerSettings()
{
}

bool MIPI_DSI_LP_AnalyzerSettings::SetSettingsFromInterfaces()
{
	mPosChannel = mSettingChannelP->GetChannel();
	mNegChannel = mSettingChannelN->GetChannel();

	if (mPosChannel == mNegChannel)
	{
		SetErrorText("D+ and D- can't be assigned to the same input.");
		return false;
	}

	ClearChannels();
	AddChannel(mPosChannel, "D+", true);
	AddChannel(mNegChannel, "D-", true);

	return true;
}

void MIPI_DSI_LP_AnalyzerSettings::UpdateInterfacesFromSettings()
{
	mSettingChannelP->SetChannel(mPosChannel);
	mSettingChannelN->SetChannel(mNegChannel);
}

void MIPI_DSI_LP_AnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString(settings);

	const char* name_string;	// The first thing in the archive is the name of the protocol analyzer that the data belongs to.
	text_archive >> &name_string;
	if (strcmp(name_string, "Saleae_MIPI_DSI_LP_Analyzer") != 0)
		AnalyzerHelpers::Assert("Saleae_MIPI_DSI_LP_Analyzer: Provided with a settings string that doesn't belong to us;");

	text_archive >> mPosChannel;
	text_archive >> mNegChannel;

	ClearChannels();
	AddChannel(mPosChannel, "D+", true);
	AddChannel(mNegChannel, "D-", true);

	UpdateInterfacesFromSettings();
}

const char* MIPI_DSI_LP_AnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << "Saleae_MIPI_DSI_LP_Analyzer";
	text_archive << mPosChannel;
	text_archive << mNegChannel;

	return SetReturnString(text_archive.GetString());
}
