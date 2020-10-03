#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <gq/gconfitem.h>
#include "mobilehotspotconfiguration.h"

const QString MobileHotspotConfiguration::ENCRYPTION_NONE = QString("None");
const QString MobileHotspotConfiguration::ENCRYPTION_WEP = QString("WEP");
const QString MobileHotspotConfiguration::INTERFACE_GPRS = QString("gprs0");
const QString MobileHotspotConfiguration::INTERFACE_USB = QString("usb0");
const QString MobileHotspotConfiguration::INTERFACE_WLAN = QString("wlan0");


MobileHotspotConfiguration::MobileHotspotConfiguration(QObject *parent)
	:	QObject(parent){
	internetEnabled = true;
}

MobileHotspotConfiguration::~MobileHotspotConfiguration(){
}

void MobileHotspotConfiguration::load(){
	GConfItem *item;

	item = new GConfItem(GCONF_LANGUAGE);
	language = item->value(NO_LANGUAGE_SELECTED).toString();
	delete item;

	item = new GConfItem(GCONF_APNAME);
	hotspotName = item->value("N900 Hotspot").toString();
	delete item;

	item = new GConfItem(GCONF_ENCRYPTION);
	encryptionType = item->value(ENCRYPTION_NONE).toString();
	delete item;

	item = new GConfItem(GCONF_KEY);
	encryptionKey = item->value("0000000000000").toString();
	delete item;

	item = new GConfItem(GCONF_WLANMAC);
	wlanCustomMAC = item->value("02:11:22:33:44:55").toString();
	delete item;

	item = new GConfItem(GCONF_WLANMACTYPE);
	QString wlanmactype = item->value("normal").toString();
	if(wlanmactype == "custom")
		wlanTypeMAC = MAC_CUSTOM;
	else if(wlanmactype == "random")
		wlanTypeMAC = MAC_RANDOM;
	else
		wlanTypeMAC = MAC_NORMAL;
	delete item;

	item = new GConfItem(GCONF_WLANDRIVER);
	cycleWlanDriver = item->value(false).toBool();
	delete item;

	item = new GConfItem(GCONF_INTERFACE);
	lanInterface = item->value(INTERFACE_WLAN).toString();
	delete item;

	item = new GConfItem(GCONF_APN);
	internetAPName = item->value("").toString();
	delete item;

	item = new GConfItem(GCONF_APTYPE);
	internetAPType = item->value("").toString();
	delete item;

	item = new GConfItem(GCONF_APID);
	internetAPID = item->value("").toString();
	delete item;

	item = new GConfItem(GCONF_NETWORK);
	lanNetwork = item->value("192.168.254.").toString();
	delete item;

	item = new GConfItem(GCONF_INTERNET);
	internetEnabled = item->value(true).toBool();
	delete item;

	item = new GConfItem(GCONF_2G3G);
	QString enf = item->value("ignore").toString();
	if(enf == "2g")
		enforcement2g3g = ENFORCEMENT_2G3G_2G;
	else if(enf == "3g")
		enforcement2g3g = ENFORCEMENT_2G3G_3G;
	else if(enf == "2g3g")
		enforcement2g3g = ENFORCEMENT_2G3G_DOUBLE;
	else
		enforcement2g3g = ENFORCEMENT_2G3G_IGNORE;
	delete item;

	loadUSBMAC();
}

void MobileHotspotConfiguration::save(){
	GConfItem *item;

	item = new GConfItem(GCONF_LANGUAGE);
	item->set(language);
	delete item;

	item = new GConfItem(GCONF_APNAME);
	item->set(hotspotName);
	delete item;

	item = new GConfItem(GCONF_ENCRYPTION);
	item->set(encryptionType);
	delete item;

	item = new GConfItem(GCONF_KEY);
	item->set(encryptionKey);
	delete item;

	item = new GConfItem(GCONF_WLANMAC);
	item->set(wlanCustomMAC);
	delete item;

	item = new GConfItem(GCONF_WLANMACTYPE);
	if(wlanTypeMAC == MAC_CUSTOM)
		item->set("custom");
	else if(wlanTypeMAC == MAC_RANDOM)
		item->set("random");
	else
		item->set("normal");
	delete item;

	item = new GConfItem(GCONF_WLANDRIVER);
	item->set(cycleWlanDriver);
	delete item;

	item = new GConfItem(GCONF_INTERFACE);
	item->set(lanInterface);
	delete item;

	item = new GConfItem(GCONF_APN);
	item->set(internetAPName);
	delete item;

	item = new GConfItem(GCONF_APTYPE);
	item->set(internetAPType);
	delete item;

	item = new GConfItem(GCONF_APID);
	item->set(internetAPID);
	delete item;

	item = new GConfItem(GCONF_NETWORK);
	item->set(lanNetwork);
	delete item;

	item = new GConfItem(GCONF_INTERNET);
	item->set(internetEnabled);
	delete item;

	item = new GConfItem(GCONF_2G3G);
	if(enforcement2g3g == ENFORCEMENT_2G3G_2G)
		item->set("2g");
	else if(enforcement2g3g == ENFORCEMENT_2G3G_3G)
		item->set("3g");
	else if(enforcement2g3g == ENFORCEMENT_2G3G_DOUBLE)
		item->set("2g3g");
	else
		item->set("ignore");
	delete item;

	saveUSBMAC();
}

QString MobileHotspotConfiguration::internetInterface() const{
	if(internetAPType == APN_TYPE_INFRA || internetAPType == APN_TYPE_ADHOC)
		return INTERFACE_WLAN;
	else
		return INTERFACE_GPRS;
}

void MobileHotspotConfiguration::loadUSBMAC(){
	QFile file(FILE_USBMAC);
	try{
		if(! file.open(QIODevice::ReadOnly | QIODevice::Text))
			throw (QString("Cannot open file ") + FILE_USBMAC);
		usbMAC = QString(file.readAll()).trimmed();
		file.close();
	}
	catch(QString fail){
		qDebug("WARNING : Was not able to load the USB MAC address (%s), using a new one", fail.toLatin1().data());
		usbMAC = newMAC();
		saveUSBMAC();
	}
}

QString MobileHotspotConfiguration::newMAC(){
	QString result("02:");
	qsrand(QDateTime::currentDateTime().toTime_t());
	for(int i = 0; i < 5; i++){
		if(i > 0)
			result += ":";
		int number1 = qrand() % 16;
		int number2 = qrand() % 16;
		result += QString("%1").arg(number1, 0, 16);
		result += QString("%1").arg(number2, 0, 16);
	}
	result = result.toLower();
	return result;
}

void MobileHotspotConfiguration::saveUSBMAC(){
	QFile file(FILE_USBMAC);
	try{
		if(! file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
			throw (QString("Cannot open file ") + FILE_USBMAC);
		file.write(usbMAC.toLatin1());
		file.close();
	}
	catch(QString fail){
		qDebug("WARNING : Was not able to save the USB MAC address (%s)", fail.toLatin1().data());
	}
}
