#ifndef PRIVOXYHELPER_H
#define PRIVOXYHELPER_H

#include <QtCore/QProcess>
#include "../../qtMobileHotspot/src/mobilehotspotconfiguration.h"
#include "pluginconfiguration.h"

#define PRIVOXY_FILE            "./util/qtmhprivoxy/privoxy.executable"
#define CONFIGURATION_FILE      "./util/qtmhprivoxy/privoxy.config"
#define CONFIGURATION_FILE_QTMH "./util/qtmhprivoxy/privoxy.config.qtmh"
#define ACTION_FILE             "./util/qtmhprivoxy/qtmh.action"
#define ACTION_FILE_QTMH        "./util/qtmhprivoxy/qtmh.action.qtmh"
#define MICROB                  "Mozilla/5.0 (X11; U; Linux armv7l; en-GB; rv:1.9.2b6pre) Gecko/20100318 Firefox/3.5 Maemo Browser 1.7.4.8 RX-51 N900"
#define IPHONE                  "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16"


class PrivoxyHelper{

public:
	PrivoxyHelper();
	~PrivoxyHelper();
	static int generateConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf);
	static int deleteConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf);
	static int generateActionFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf);
	static int deleteActionFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf);
	static int runPrivoxy(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process);
	static int stopPrivoxy(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process);
	static int setIptables(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf);
};

#endif // PRIVOXYHELPER_H
