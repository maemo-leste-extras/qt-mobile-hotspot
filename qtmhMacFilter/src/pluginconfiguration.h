#ifndef PLUGINCONFIGURATION_H
#define PLUGINCONFIGURATION_H

#include <QtCore/QStringList>
#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"


enum MacFilterMode{
	MODE_ALLOW,
	MODE_DENY
};


class PluginConfiguration{

public:
	PluginConfiguration(MobileHotspotPluginHelper *helper);
	~PluginConfiguration();
	void save();
	void load();
	void reset();

	MacFilterMode filterMode;
	QStringList mactable;

private:
	MobileHotspotPluginHelper *helper; // Helper for this plugin
};


#endif // PLUGINCONFIGURATION_H

