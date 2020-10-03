#ifndef PLUGIN_H
#define PLUGIN_H

#include <QtCore/QObject>
#include <QtGui/QMainWindow>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QIcon>
#include <QtCore/QTranslator>
#include <QtCore/QProcess>
#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"
#include "../../qtMobileHotspot/src/mobilehotspotplugin.h"
#include "../../qtMobileHotspot/src/mobilehotspotconfiguration.h"
#include "plugingui.h"
#include "pluginconfiguration.h"


class Plugin : public QObject, public MobileHotspotPlugin{

	Q_OBJECT
	Q_INTERFACES(MobileHotspotPlugin)

public:
	Plugin();
	~Plugin();

	QString displayedName();
	QString displayedAuthor();
	QString displayedContact();
	QString displayedDescription();
	QIcon displayedIcon();

	QString name();
	int order();
	QStringList provided();
	bool allowConfigureStopped();
	bool allowConfigureRunning();
	bool allowEnableDisableStopped();
	bool allowEnableDisableRunning();

	bool isPluginEnabled();
	void setPluginEnabled(bool enabled, const MobileHotspotConfiguration *configuration, bool running);

	void configure(const QMainWindow *parentWindow, const MobileHotspotConfiguration *configuration, bool running, bool reset);
	bool provide(QString service, const MobileHotspotConfiguration *configuration);
	bool unprovide(QString service, const MobileHotspotConfiguration *configuration, bool successStarting);
	void beforeStarting(const MobileHotspotConfiguration *configuration);
	void afterStarting(const MobileHotspotConfiguration *configuration, bool successStarting);
	void beforeStopping(const MobileHotspotConfiguration *configuration, bool successStarting);
	void afterStopping(const MobileHotspotConfiguration *configuration, bool successStarting, bool successFinishing);

	QTranslator *translator();

private:
	MobileHotspotPluginHelper *helper; // Helper for this plugin
	PluginConfiguration *conf; // Configuration of this plugin
	QTranslator translater; // Translator of this plugin (even if there are no translations)
	PluginGUI *gui; // Configuration GUI
	const MobileHotspotConfiguration *configuration; // Qt Mobile Hotspot configuration
	bool running; // Is the hotspot running ?
	bool successStarting; // Was hotspot startup successful ?
	bool successFinishing; // Was hotspot shutdown successful ?

	QProcess *privoxyProcess;

private slots:
	void configureFinished(); // Callback at GUI closing
};


#endif // PLUGIN_H
