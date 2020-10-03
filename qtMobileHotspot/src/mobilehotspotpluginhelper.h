#ifndef MOBILEHOTSPOTPLUGINHELPER_H
#define MOBILEHOTSPOTPLUGINHELPER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include "mobilehotspotplugin.h"


#define GCONF_PLUGIN_ENABLED_SUFFIX ".internal.enabled"


class MobileHotspotPluginHelper{

public:
	MobileHotspotPluginHelper(MobileHotspotPlugin *plug);
	~MobileHotspotPluginHelper();

	void save(QString name, QVariant data);
	QVariant load(QString name, QVariant def);
	void deleteConfiguration();
	void setPluginEnabled(bool enabled);
	bool isPluginEnabled();
	void installTranslator();
	void removeTranslator();

private:
	QString gconfKey();
	QString gconfEnabledKey();

	MobileHotspotPlugin *plugin;
	bool enabled;
};


#endif // MOBILEHOTSPOTPLUGINHELPER_H

