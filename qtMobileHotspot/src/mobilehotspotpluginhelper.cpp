#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QListIterator>
#include <QtCore/QVariant>
#include <gq/gconfitem.h>
#include "mobilehotspotconfiguration.h"
#include "mobilehotspotplugin.h"
#include "mobilehotspotpluginhelper.h"


MobileHotspotPluginHelper::MobileHotspotPluginHelper(MobileHotspotPlugin *plug){
	plugin = plug;

	GConfItem item(gconfEnabledKey());
	enabled = item.value(false).toBool();
}

MobileHotspotPluginHelper::~MobileHotspotPluginHelper(){
}

QString MobileHotspotPluginHelper::gconfKey(){
	return QString(GCONF) + "/" + plugin->name();
}

QString MobileHotspotPluginHelper::gconfEnabledKey(){
	return QString(GCONF) + "/" + plugin->name() + GCONF_PLUGIN_ENABLED_SUFFIX;
}

void MobileHotspotPluginHelper::save(QString name, QVariant value){
	/** Supports (GConfItem) :
	 * - QVariant::Int, QVariant::Double, QVariant::Bool
	 * - QVariant::String (UTF8 only)
	 * - QVariant::StringList
	 * - QVariant::List
	 */
	GConfItem item(gconfKey() + "/" + name);
	item.set(value);
}

QVariant MobileHotspotPluginHelper::load(QString name, QVariant def){
	/** Supports (GConfItem) :
	 * - QVariant::Int, QVariant::Double, QVariant::Bool
	 * - QVariant::String (UTF8 only)
	 * - QVariant::StringList
	 * - QVariant::List
	 */
	GConfItem item(gconfKey() + "/" + name);
	return item.value(def);
}

void MobileHotspotPluginHelper::deleteConfiguration(){
	GConfItem item(gconfKey());
	QList<QString> entries = item.listEntries();
	QListIterator<QString> iterator(entries);
	while(iterator.hasNext()){
		QString entryName = iterator.next();
		GConfItem entryItem(entryName);
		entryItem.unset();
	}
}

bool MobileHotspotPluginHelper::isPluginEnabled(){
	return enabled;
}

void MobileHotspotPluginHelper::setPluginEnabled(bool enabled){
	this->enabled = enabled;

	GConfItem item(gconfEnabledKey());
	item.set(enabled);
}

void MobileHotspotPluginHelper::installTranslator(){
	qApp->installTranslator(plugin->translator());
}

void MobileHotspotPluginHelper::removeTranslator(){
	qApp->removeTranslator(plugin->translator());
}
