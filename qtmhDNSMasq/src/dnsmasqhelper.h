#ifndef DNSMASQHELPER_H
#define DNSMASQHELPER_H

#include <QtCore/QProcess>
#include "../../qtMobileHotspot/src/mobilehotspotconfiguration.h"
#include "pluginconfiguration.h"

#define CONFIGURATION_FILE      "./util/qtmhdnsmasq/dnsmasq.conf"
#define CONFIGURATION_FILE_QTMH "./util/qtmhdnsmasq/dnsmasq.conf.qtmh"


class DNSMasqHelper{

public:
	DNSMasqHelper();
	~DNSMasqHelper();
	static int generateConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf);
	static int deleteConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf);
	static int runDNSMasq(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process);
	static int stopDNSMasq(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process);
};

#endif // DNSMASQHELPER_H
