#ifndef MIPI_DSI_LP__ANALYZER_SETTINGS
#define MIPI_DSI_LP__ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class MIPI_DSI_LP_AnalyzerSettings : public AnalyzerSettings
{
public:
	MIPI_DSI_LP_AnalyzerSettings();
	virtual ~MIPI_DSI_LP_AnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	Channel mPosChannel, mNegChannel;

protected:
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mSettingChannelP, mSettingChannelN;
};

#endif //MIPI_DSI_LP__ANALYZER_SETTINGS
