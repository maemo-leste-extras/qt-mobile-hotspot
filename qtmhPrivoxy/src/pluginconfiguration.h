#ifndef PLUGINCONFIGURATION_H
#define PLUGINCONFIGURATION_H

#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"


enum UserAgent{
	UA_NOCHANGE,
	UA_MICROB,
	UA_IPHONE,
	UA_CUSTOM
};

class PluginConfiguration{

public:
	PluginConfiguration();
	PluginConfiguration(MobileHotspotPluginHelper *helper);
	~PluginConfiguration();
	void save();
	void load();
	void reset();

	int port;
	QString ports;
	UserAgent ua;
	QString customUA;
	bool defaultActions;
	bool forward;
	QString forwardIP;
	int forwardPort;

private:
	MobileHotspotPluginHelper *helper; // Helper for this plugin
};


#endif // PLUGINCONFIGURATION_H

