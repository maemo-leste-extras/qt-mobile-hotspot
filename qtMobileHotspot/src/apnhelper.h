#ifndef APNHELPER_H
#define APNHELPER_H

#include <QtCore/QList>
#include <QtCore/QStringList>


#define GCONF_APN_LIST           "/system/osso/connectivity/IAP"
#define GCONF_APN_TYPE           "/type"
#define GCONF_APN_TYPE_GPRS      "GPRS"
#define GCONF_APN_TYPE_INFRA     "WLAN_INFRA"
#define GCONF_APN_TYPE_ADHOC     "WLAN_ADHOC"
#define GCONF_APN_NAME           "/name"
#define GCONF_APN_POWER          "/wlan_powersave"
#define GCONF_APN_POWER_NOTFOUND ((int)-2)
#define GCONF_APN_POWER_INVALID  ((int)-1)
#define GCONF_APN_POWER_NOECO    ((int)1)
#define GCONF_APN_POWER_ECO      ((int)2)
#define GCONF_APN_POWER_ECOMAX   ((int)4)
#define GCONF_SCANINTERVAL       "/system/osso/connectivity/network_type/search_interval"


class APNHelper
{

public:
	APNHelper();
    ~APNHelper();
    static QList<QStringList> apnList(bool gprsOnly = false);
    static void setScanInterval(int newValue);
    static int scanInterval();
    static void setAPNPowerSave(QString apid, int level);
    static int apnPowerSave(QString apid);

private:
	static QList<QStringList> sortAPNList(QList<QStringList> apns);
};

#endif // APNHELPER_H
