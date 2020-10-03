#ifndef PLUGINCONFIGURATION_H
#define PLUGINCONFIGURATION_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"


class PluginConfiguration{

public:
	PluginConfiguration(MobileHotspotPluginHelper *helper);
	~PluginConfiguration();
	void save();
	void load();
	void reset();

	QString domainName;
	bool noHosts;
	QStringList dnsTable;
	QStringList dhcpTable;
	QString dhcpFrom;
	QString dhcpTo;

private:
	MobileHotspotPluginHelper *helper; // Helper for this plugin
};


#endif // PLUGINCONFIGURATION_H

