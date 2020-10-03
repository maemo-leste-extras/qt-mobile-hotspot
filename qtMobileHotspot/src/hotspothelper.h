#ifndef HOTSPOTHELPER_H
#define HOTSPOTHELPER_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include "dbushelper.h"
#include "mobilehotspotconfiguration.h"


#define MAX_TRIES (15)
#define SYSTEM_MODULES "nf_conntrack, nf_defrag_ipv4, nf_conntrack_ipv4, x_tables, xt_mac, ip_tables, iptable_mangle, iptable_filter, nf_nat, iptable_nat, ipt_MASQUERADE" // Credits to Rambo


class HotspotHelper
{

public:
	HotspotHelper();
    ~HotspotHelper();
    static int loadSystemModules(QStringList *alreadyLoadedModules);
    static int unloadSystemModules(QStringList &alreadyLoadedModules);
    static int disableInternetConnection(MobileHotspotConfiguration &configuration);
    static int enableInternetConnection(MobileHotspotConfiguration &configuration);
    static int checkUSBMode();
    static int loadStandardUSB();
    static void unloadStandardUSB();
    static int loadNokiaUSB();
    static void unloadNokiaUSB();
    static int prepareUSBInterface(QString usbMAC, QString lanNetwork);
    static int prepareWifiInterface(MobileHotspotConfiguration &configuration, QString *oldMAC);
    static int unprepareUSBInterface();
    static int unprepareWifiInterface(MobileHotspotConfiguration &configuration, QString &oldMAC);
    static int startDnsMasq(MobileHotspotConfiguration &configuration, QProcess *dnsmasqProcess);
    static int stopDnsMasq(MobileHotspotConfiguration &configuration, QProcess *dnsmasqProcess);
    static int setIPTables(MobileHotspotConfiguration &configuration, int *ipforward);
    static int unsetIPTables(MobileHotspotConfiguration &configuration, int ipforward);

private:
	static int loadModule(QString moduleName);
	static int unloadModule(QString moduleName);
	static QStringList getSystemModules();
	static int enableUSBInterface(QString lanNetwork);
	static int getInterfaceMAC(QString interface, QString *mac);
	static int setInterfaceMAC(QString interface, QString mac);
	static QString randomMAC();
	static int disableUSBInterface();
	static int enableWifiInterface(MobileHotspotConfiguration &configuration);
	static int disableWifiInterface(MobileHotspotConfiguration &configuration);
	static int setPowerManagement(MobileHotspotConfiguration &configuration, bool management);
	static int setEncryption(MobileHotspotConfiguration &configuration, bool encryption);
	static int setAdhocMode(MobileHotspotConfiguration &configuration, bool adhoc);
	static QString keyToHex(QString key);
	static int setESSID(MobileHotspotConfiguration &configuration, bool useESSID);
	static int setWifiNetwork(MobileHotspotConfiguration &configuration);
	static int cycleWLANDriver(MobileHotspotConfiguration &configuration);
};

#endif // HOTSPOTHELPER_H
