#ifndef MOBILEHOTSPOTCONFIGURATION_H
#define MOBILEHOTSPOTCONFIGURATION_H

#include <QtCore/QObject>
#include <QtCore/QString>


#define GCONF "/apps/qtmobilehotspot"
#define GCONF_LANGUAGE GCONF "/lang"
#define GCONF_APNAME GCONF "/name"
#define GCONF_ENCRYPTION GCONF "/encryption"
#define GCONF_KEY GCONF "/key"
#define GCONF_WLANMACTYPE GCONF "/mactype"
#define GCONF_WLANMAC GCONF "/mac"
#define GCONF_WLANDRIVER GCONF "/wlandriver"
#define GCONF_INTERFACE GCONF "/interface"
#define GCONF_APN GCONF "/apn"
#define GCONF_APID GCONF "/apid"
#define GCONF_APTYPE GCONF "/aptype"
#define GCONF_NETWORK GCONF "/network"
#define GCONF_INTERNET GCONF "/internet"
#define GCONF_2G3G	GCONF "/2g3g"

#define PERSISTENT_FOLDER "/home/user/.qtmobilehotspot"
#define FILE_USBMAC PERSISTENT_FOLDER "/usb.mac"

#define NO_LANGUAGE_SELECTED "?"
#define DEFAULT_LANGUAGE ""

#define APN_TYPE_INFRA     "WLAN_INFRA" // = GCONF_APN_TYPE_INFRA from APNHelper
#define APN_TYPE_ADHOC     "WLAN_ADHOC" // = GCONF_APN_TYPE_ADHOC from APNHelper
#define APN_TYPE_GPRS      "GPRS" // = GCONF_APN_TYPE_GPRS from APNHelper

enum Enforcement_2G3G{
	ENFORCEMENT_2G3G_2G,
	ENFORCEMENT_2G3G_3G,
	ENFORCEMENT_2G3G_DOUBLE,
	ENFORCEMENT_2G3G_IGNORE
};

enum MacType{
	MAC_NORMAL,
	MAC_CUSTOM,
	MAC_RANDOM
};

class MobileHotspotConfiguration : public QObject
{
    Q_OBJECT

public:
	static const QString ENCRYPTION_NONE;
	static const QString ENCRYPTION_WEP;
	static const QString INTERFACE_WLAN;
	static const QString INTERFACE_USB;
	static const QString INTERFACE_GPRS;

	QString language, hotspotName, encryptionType, encryptionKey, wlanCustomMAC, lanInterface, lanNetwork, internetAPName, internetAPType, internetAPID, usbMAC;
	bool internetEnabled, cycleWlanDriver;
	Enforcement_2G3G enforcement2g3g;
	MacType wlanTypeMAC;

public:
	MobileHotspotConfiguration(QObject *parent = 0);
    ~MobileHotspotConfiguration();
    void load();
    void save();
    QString internetInterface() const;

private:
	QString newMAC();
	void saveUSBMAC();
	void loadUSBMAC();
};

#endif // MOBILEHOTSPOTCONFIGURATION_H
