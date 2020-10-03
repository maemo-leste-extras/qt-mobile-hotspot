#ifndef PLUGINCONFIGURATION_H
#define PLUGINCONFIGURATION_H

#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"


class PluginConfiguration{

public:
	PluginConfiguration(MobileHotspotPluginHelper *helper);
	~PluginConfiguration();
	void save();
	void load();
	void reset();

private:
	MobileHotspotPluginHelper *helper; // Helper for this plugin
};


#endif // PLUGINCONFIGURATION_H

