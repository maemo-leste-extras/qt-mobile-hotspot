#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QListIterator>
#include <QtCore/QByteArray>
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QIODevice>
#include <QtCore/QTextStream>
#include "mobilehotspotconfiguration.h"
#include "commandhelper.h"
#include "dbushelper.h"
#include "hotspothelper.h"


HotspotHelper::HotspotHelper(){
}

HotspotHelper::~HotspotHelper(){
}

int HotspotHelper::loadSystemModules(QStringList *alreadyLoadedModules){
	/* 0=ok, -1=fail */
	qDebug("** Loading system modules **");
	int result = 0, cmdret;
	QStringList mods = getSystemModules();
	QString loadedModules;
	CommandHelper::executeCommandWithStdOut("lsmod", &cmdret, &loadedModules);
	QListIterator<QString> iterator(mods);
	while(iterator.hasNext()){
		QString mod = iterator.next();
		if(loadedModules.count(mod) > 0){
			qDebug( (mod + " already loaded").toAscii().data() );
			alreadyLoadedModules->append(mod);
		}
		else{
			qDebug( (QString("Loading ") + mod).toAscii().data() );
			int ret = loadModule(mod);
			if(ret != 0){
				qDebug( (QString("FAILURE WHEN LOADING ") + mod).toAscii().data());
				result = -1;
			}
		}
	}
	return result;
}

QStringList HotspotHelper::getSystemModules(){
	QString mods(SYSTEM_MODULES);
	QStringList list = mods.replace(" ", "").split(',');
	return list;
}

int HotspotHelper::loadModule(QString moduleName){
	/* 0=ok, -1=fail */
	QString cmd("modprobe ");
	cmd += moduleName;
	return CommandHelper::executeCommandOld(cmd) == 0 ? 0 : -1;
}

int HotspotHelper::unloadSystemModules(QStringList &alreadyLoadedModules){
	/* 0=ok, -1=fail */
	qDebug("** Unloading system modules **");
	int result = 0;
	QStringList mods = getSystemModules();
	QListIterator<QString> iterator(mods);
	iterator.toBack();
	while(iterator.hasPrevious()){
		QString mod = iterator.previous();
		if(alreadyLoadedModules.contains(mod))
			qDebug( (QString("Not unloading (was already loaded at startup) ") + mod).toAscii().data());
		else{
			qDebug((QString("Unloading ") + mod).toAscii().data());
			int ret = unloadModule(mod);
			if(ret != 0){
				qDebug( (QString("FAILURE WHEN UNLOADING ") + mod).toAscii().data());
				result = -1;
			}
		}
	}
	return result;
}

int HotspotHelper::unloadModule(QString moduleName){
	/* 0=ok, -1=fail */
	QString cmd("rmmod "); 	cmd += moduleName;
	return CommandHelper::executeCommandOld(cmd) == 0 ? 0 : -1;
}

int HotspotHelper::disableInternetConnection(MobileHotspotConfiguration &configuration){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Disabling internet connection **");
	int ret, tries;
	bool trying;
	ret = DBUSHelper::internetDisconnect();
	if(ret != 0){
		qDebug("FAILURE : Cannot disconnect internet connection");
		return -1;
	}
	tries = 0; trying = true;
	while(trying && tries < MAX_TRIES){
		tries++;
		if(DBUSHelper::internetConnectionsCount() == 0)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow to disconnect internet connection");
		return -2;
	}
	return 0;
}

int HotspotHelper::enableInternetConnection(MobileHotspotConfiguration &configuration){
	/* -1=fail, -2=timeout, 0=ok */
	int ret, tries;
	bool trying;
	qDebug( (((QString("** Enabling internet connection to ") + configuration.internetAPName) + " (") + (configuration.internetAPID + ") **")).toAscii().data() );
	ret = DBUSHelper::internetConnect(configuration.internetAPID);
	if(ret != 0){
		qDebug("Error while connecting to the chosen APN");
		return -1;
	}
	tries = 0; trying = true;
	while(trying && tries < MAX_TRIES){
		tries++;
		if(DBUSHelper::internetConnectionsCount() == 1)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("Error while connecting to the chosen APN : system was too slow");
		return -2;
	}
	return 0;
}

int HotspotHelper::prepareUSBInterface(QString usbMAC, QString lanNetwork){
	/* -1=fail, 0=ok */
	qDebug("** Preparing USB interface **");
	int ret;

	/** Disable USB interface - may fail, not fatal unless timeout **/
	ret = disableUSBInterface();
	if(ret == -2){
		qDebug("Error (timeout) while disabling the USB interface");
		return -1;
	}
	else if(ret != 0)
		qDebug("WARNING :  Was not able to disable the USB interface");

	/** Change USB MAC if necessary **/
	if(usbMAC != ""){
		ret = setInterfaceMAC(MobileHotspotConfiguration::INTERFACE_USB, usbMAC);
		if(ret == -2){
			qDebug("Error (timeout) while changing the USB MAC");
			return -1;
		}
		else if(ret != 0)
			qDebug("WARNING : Was not able to change the USB MAC");
	}

	/** Enable USB interface **/
	ret = enableUSBInterface(lanNetwork);
	if(ret != 0){
		qDebug("Error while enabling the USB interface");
		return -1;
	}

	return 0;
}

int HotspotHelper::setInterfaceMAC(QString interface, QString mac){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("*** Changing MAC ***");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/ifconfig "); cmd += interface; cmd += " hw ether "; cmd += mac;
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot change MAC");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut(QString("/sbin/ifconfig ") + interface, &ret, &res);
		if(res.toLower().count(mac.toLower()) > 0)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow changing MAC");
		return -2;
	}
	return 0;
}

int HotspotHelper::getInterfaceMAC(QString interface, QString *mac){
	/* -1=fail, 0=ok */
	qDebug("*** Getting MAC ***");
	bool res;
	int ret;
	QString stdout;
	res = CommandHelper::executeCommandWithStdOut("/bin/sh -c \"ifconfig " + interface + " | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'\"" , &ret, &stdout);
	if( (! res) || ret != 0){
		qDebug("FAILURE : Cannot get MAC");
		return -1;
	}
	*mac = stdout.trimmed();
	return 0;
}

QString HotspotHelper::randomMAC(){
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

int HotspotHelper::checkUSBMode(){
	/* -1=fail, 0=ok */
	qDebug("** Checking USB mode **");
	QString res; int cmdret;
	CommandHelper::executeCommandWithStdOut("cat /proc/driver/musb_hdrc", &cmdret, &res);
	res = res.toLower();
	if(res.count("gadget driver: (none)") < 1 ){
		qDebug("FAILURE : USB should be free of any module (=no USB-mode) but it is still used");
		return -1;
	}
	return 0;
}

int HotspotHelper::loadStandardUSB(){
	/* -1=fail, 0=ok */
	qDebug("** Loading g_ether **");
	int ret;
	ret = loadModule("g_ether");
	if(ret != 0){
		qDebug("WARNING : Was not able to load g_ether, it is supposed to be provided by recents power-kernels (>=v46)");
		return -1;
	}
	return 0;
}

void HotspotHelper::unloadStandardUSB(){
	qDebug("** Unloading g_ether **");
	int ret;
	ret = unloadModule("g_ether");
	if(ret != 0)
		qDebug("WARNING : Was not able to unload g_ether");
}

int HotspotHelper::loadNokiaUSB(){
	/* -1=fail, 0=ok */
	qDebug("** Loading g_file_storage **");
	int ret;
	ret = loadModule("g_file_storage");
	if(ret != 0){
		qDebug("WARNING : Was not able to load g_file_storage");
		return -1;
	}
	return 0;
}

void HotspotHelper::unloadNokiaUSB(){
	qDebug("** Unloading g_nokia & g_file_storage **");
	int ret;
	ret = unloadModule("g_nokia");
	if(ret != 0)
		qDebug("WARNING : Was not able to unload g_nokia");
	ret = unloadModule("g_file_storage");
	if(ret != 0)
		qDebug("WARNING : Was not able to unload g_file_storage");
}

int HotspotHelper::disableUSBInterface(){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Disabling USB interface **");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/ifconfig "); cmd += MobileHotspotConfiguration::INTERFACE_USB; cmd += " down";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot disable USB interface");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut("/sbin/ifconfig", &ret, &res);
		if(res.count(MobileHotspotConfiguration::INTERFACE_USB) < 1)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow disabling USB interface");
		return -2;
	}
	return 0;
}

int HotspotHelper::enableUSBInterface(QString lanNetwork){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Enabling USB interface **");
	int tries, ret;
	bool cmdret, trying;
	QString cmd("/sbin/ifconfig "); cmd += MobileHotspotConfiguration::INTERFACE_USB; cmd += " ";
	cmd += lanNetwork; cmd += "1 netmask 255.255.255.0 up";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot enable USB interface");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut("/sbin/ifconfig", &ret, &res);
		if(res.count(MobileHotspotConfiguration::INTERFACE_USB) > 0)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow enabling USB interface");
		return -2;
	}
	return 0;
}

int HotspotHelper::prepareWifiInterface(MobileHotspotConfiguration &configuration, QString *oldMAC){
	/* -1=fail, 0=ok */
	qDebug("** Preparing Wifi interface **");
	int ret;

	/** Disable Wifi interface - may fail, not fatal unless timeout **/
	ret = disableWifiInterface(configuration);
	if(ret == -2){
		qDebug("Error (timeout) while disabling the Wifi interface");
		return -1;
	}
	else if(ret != 0)
		qDebug("WARNING :  Was not able to disable the Wifi interface");

	if(configuration.cycleWlanDriver){
		/** Cycle WLAN driver **/
		ret = cycleWLANDriver(configuration);
		if(ret != 0){
			qDebug("Error while cycling the WLAN driver");
			return -1;
		}
	}

	/** Save MAC address **/
	ret = getInterfaceMAC(configuration.INTERFACE_WLAN, oldMAC);
	if(ret != 0)
		qDebug("WARNING : Was not able to save the WLAN MAC");

	/** MAC changing if needed **/
	if(configuration.wlanTypeMAC != MAC_NORMAL){
		ret = setInterfaceMAC(configuration.INTERFACE_WLAN, configuration.wlanTypeMAC == MAC_CUSTOM ? configuration.wlanCustomMAC : randomMAC());
		if(ret != 0){
			qDebug("Error while changing the WLAN MAC");
			return -1;
		}
	}

	/** Power saving - May not work on adhoc, but who knows **/
	/** Switch to managed mode (to be able to change power management) - may fail, not fatal unless timeout **/
	ret = setAdhocMode(configuration, false);
	if(ret == -2){
		qDebug("Error (timeout) while enabling managed mode");
		return -1;
	}
	else if(ret != 0)
		qDebug("WARNING : Was not able to enable managed mode");

	/** Disable power management - may fail, not fatal unless timeout **/
	ret = setPowerManagement(configuration, false);
	if(ret == -2){
		qDebug("Error (timeout) while disabling power management");
		return -1;
	}
	else if(ret != 0)
		qDebug("WARNING : Was not able to disable power management");

	/** Enable Ad-Hoc mode - may fail, not fatal unless timeout **/
	ret = setAdhocMode(configuration, true);
	if(ret == -2){
		qDebug("Error (timeout) while enabling ad-hoc mode");
		return -1;
	}
	else if(ret != 0)
		qDebug("WARNING : Was not able to enable ad-hoc mode");

	/** Set encryption **/
	ret = setEncryption(configuration, configuration.encryptionType == configuration.ENCRYPTION_WEP);
	if(ret != 0){
		qDebug("Error while changing the encryption");
		return -1;
	}

	/** Enable Wifi interface **/
	ret = enableWifiInterface(configuration);
	if(ret != 0){
		qDebug("Error while enabling the Wifi interface");
		return -1;
	}

	/** Set ESSID **/
	ret = setESSID(configuration, true);
	if(ret != 0){
		qDebug("Error while setting the ESSID");
		return -1;
	}

	/** Set network IP **/
	ret = setWifiNetwork(configuration);
	if(ret != 0){
		qDebug("Error while setting wifi network");
		return -1;
	}

	return 0;
}

int HotspotHelper::disableWifiInterface(MobileHotspotConfiguration &configuration){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Disabling Wifi interface **");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/ifconfig "); cmd += " "; cmd += configuration.INTERFACE_WLAN; cmd += " down";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot disable Wifi interface");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut("/sbin/ifconfig", &ret, &res);
		if(res.count(configuration.INTERFACE_WLAN) < 1)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow disabling Wifi interface");
		return -2;
	}
	return 0;
}

int HotspotHelper::enableWifiInterface(MobileHotspotConfiguration &configuration){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Enabling Wifi interface **");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/ifconfig "); cmd += configuration.INTERFACE_WLAN; cmd += " up";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot enable Wifi interface");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut("/sbin/ifconfig", &ret, &res);
		if(res.count(configuration.INTERFACE_WLAN) > 0)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow enabling Wifi interface");
		return -2;
	}
	return 0;
}

int HotspotHelper::setPowerManagement(MobileHotspotConfiguration &configuration, bool management){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Modifying power management **");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/iwconfig "); cmd += configuration.lanInterface;
	cmd += management ? " power on" : " power off";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (ret != 0) ){
		qDebug("FAILURE : Cannot modify power management");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut(QString("/sbin/iwconfig ") + configuration.lanInterface, &ret, &res);
		res = res.toLower();
		if( (management && res.count("power management:on") > 0) || ((!management) && res.count("power management:off") > 0) )
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow modifying power management");
		return -2;
	}
	return 0;
}

int HotspotHelper::setAdhocMode(MobileHotspotConfiguration &configuration, bool adhoc){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Modifying Ad-Hoc mode **");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/iwconfig "); cmd += configuration.INTERFACE_WLAN;
	cmd += adhoc ? " mode ad-hoc" : " mode managed";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot modify ad-hoc mode");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut(QString("/sbin/iwconfig ") + configuration.INTERFACE_WLAN, &ret, &res);
		res = res.toLower();
		if( (adhoc && res.count("mode:ad-hoc") > 0) || ((!adhoc) && res.count("mode:managed") > 0) )
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow modifying ad-hoc mode");
		return -2;
	}
	return 0;
}

int HotspotHelper::setEncryption(MobileHotspotConfiguration &configuration, bool encryption){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Modifying encryption **");
	int result = 0;
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/iwconfig "); cmd += configuration.INTERFACE_WLAN;
	QString hexkey;
	if(encryption){
		qDebug("Will enable encryption");
		hexkey = keyToHex(configuration.encryptionKey).toLower();
		cmd += " key "; cmd += hexkey; cmd += " restricted";
	}
	else{
		qDebug("Will disable encryption");
		cmd += " key open";
	}
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot modify encryption");
		result = -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut(QString("/sbin/iwconfig ") + configuration.INTERFACE_WLAN, &ret, &res);
		res = res.toLower().replace("-",""); // because iwconfig displays - between digits
		if( (encryption && res.count(hexkey) > 0) || ((!encryption) && res.count("encryption key:off") > 0) )
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow modifying encryption");
		return -2;
	}
	return 0;
}

QString HotspotHelper::keyToHex(QString key){
	QString result = "";
	QByteArray data = key.toAscii();
	for(int i = 0; i < data.size(); i++){
		QString hex;
		hex.setNum((int)data.at(i), 16);
		result += hex;
	}
	return result;
}

int HotspotHelper::setESSID(MobileHotspotConfiguration &configuration, bool useESSID){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Modifying ESSID **");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/iwconfig "); cmd += configuration.INTERFACE_WLAN; cmd += " essid ";
	if(useESSID)
		cmd += QString("\"") + configuration.hotspotName + "\"";
	else
		cmd += "off";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if(! useESSID){
		if(! cmdret){
			// the iwconfig of maemo5 does return an error when we use 'iwconfig wlan0 essid off'
			// still, it does work, we really leave the IBSS
			// so in this case we just check if the command was run, and ignore its return code
			qDebug("FAILURE : Cannot leave IBSS");
			return -1;
		}
	}
	else if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot modify essid");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut(QString("/sbin/iwconfig ") + configuration.INTERFACE_WLAN, &ret, &res);
		res = res.toLower();
		QString searched("essid:\"");
		if(useESSID)
			searched += configuration.hotspotName.toLower();
		searched += "\"";
		if(res.count(searched) > 0)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow modifying essid");
		return -2;
	}
	return 0;
}

int HotspotHelper::setWifiNetwork(MobileHotspotConfiguration &configuration){
	/* -1=fail, -2=timeout, 0=ok */
	qDebug("** Setting network IP **");
	int ret, tries;
	bool cmdret, trying;
	QString cmd("/sbin/ifconfig "); cmd += configuration.INTERFACE_WLAN;
	cmd += " "; cmd += configuration.lanNetwork + "1 netmask 255.255.255.0 up";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot set nework ip");
		return -1;
	}
	trying = true; tries = 0;
	while(trying && tries < MAX_TRIES){
		tries++;
		QString res;
		CommandHelper::executeCommandWithStdOut(QString("/sbin/ifconfig ") + configuration.INTERFACE_WLAN, &ret, &res);
		res = res.toLower();
		if(res.count(configuration.lanNetwork + "1") > 0)
			trying = false;
		else
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying){
		qDebug("FAILURE : System was too slow setting nework ip");
		return -2;
	}
	return 0;
}

int HotspotHelper::unprepareUSBInterface(){
	/** -1=fail, 0=ok */
	qDebug("** Unpreparing USB interface **");
	int res = 0, ret;

	/** Disable USB interface **/
	ret = HotspotHelper::disableUSBInterface();
	if(ret != 0){
		qDebug("Error while disabling USB interface");
		res = -1;
	}

	return res;
}

int HotspotHelper::unprepareWifiInterface(MobileHotspotConfiguration &configuration, QString &oldMAC){
	/** -1=fail, 0=ok */
	qDebug("** Unpreparing Wifi interface **");
	int res = 0, ret;

	/** Leave IBSS, may fail, not fatal **/
	ret = setESSID(configuration, false);
	if(ret != 0)
		qDebug("Error while leaving the IBSS");

	/** Disable Wifi interface **/
	ret = disableWifiInterface(configuration);
	if(ret != 0){
		qDebug("Error while disabling the Wifi interface");
		res = -1;
	}

	/** Disable Ad-Hoc mode **/
	ret = setAdhocMode(configuration, false);
	if(ret != 0){
		qDebug("Error while disabling ad-hoc mode");
		res = -1;
	}

	/** Disable encryption **/
	ret = setEncryption(configuration, false);
	if(ret != 0){
		qDebug("Error while disabling encryption");
		res = -1;
	}

	/** Enable disablePowerSaving management **/
	ret = setPowerManagement(configuration, true);
	if(ret != 0){
		qDebug("Error while enabling power management");
		res = -1;
	}

	if(configuration.cycleWlanDriver){
		/** Cycle WLAN driver **/
		ret = cycleWLANDriver(configuration);
		if(ret != 0){
			qDebug("Error while cycling WLAN driver");
			res = -1;
		}
	}

	if(configuration.wlanTypeMAC != MAC_NORMAL){
		/** Restore original MAC **/
		ret = setInterfaceMAC(configuration.INTERFACE_WLAN, oldMAC);
		if(ret != 0){
			qDebug("Error while restoring WLAN MAC");
			res = -1;
		}
	}

	return res;
}

int HotspotHelper::cycleWLANDriver(MobileHotspotConfiguration &configuration){
	/** -1=fail, 0=ok **/
	qDebug("** Unloading/reloading WLAN driver **");
	int ret;
	bool res;
	QString mac = ":";
	if(getInterfaceMAC(configuration.INTERFACE_WLAN, &mac) != 0)
		qDebug("WARNING : Was not able to get WLAN MAC address, it may change without us being able to restore it");
	QString cmd = "rmmod wl12xx";
	res = CommandHelper::executeCommand(cmd , &ret);
	if( (! res) || (ret != 0) )
		qDebug("WARNING : Was not able to remove wl12xx module");
	cmd = "modprobe wl12xx";
	res = CommandHelper::executeCommand(cmd, &ret);
	if( (! res) || (ret != 0) )
		qDebug("WARNING : Was not able to load wl12xx module");
	cmd = QString("ifconfig ") + configuration.INTERFACE_WLAN + " down";
	res = CommandHelper::executeCommand(cmd, &ret);
	if( (! res) || (ret != 0) )
		qDebug("WARNING : Was not able to down the WLAN interface");
	cmd = (QString("ifconfig ") + configuration.INTERFACE_WLAN + " hw ether ") + mac;
	res = CommandHelper::executeCommand(cmd, &ret);
	if( (! res) || (ret != 0) )
		qDebug("WARNING : Was not able to restore WLAN MAC address");

	bool trying1 = true, trying2 = true;
	int tries = 0;
	while( (trying1 || trying2) && tries < MAX_TRIES ){
		tries++;
		QString s;
		CommandHelper::executeCommandWithStdOut(QString("/sbin/ifconfig ") + configuration.INTERFACE_WLAN, &ret, &s);
		if(s.toLower().count(mac.toLower()) > 0)
			trying1 = false;
		CommandHelper::executeCommandWithStdOut("lsmod", &ret, &s);
		if(s.toLower().count("wl12xx") > 0)
			trying2 = false;
		if(trying1 || trying2)
			CommandHelper::executeCommand("sleep 1");
	}
	if(trying1 || trying2){
		qDebug("FAILURE : System did not pass the WLAN driver cycle step");
		return -1;
	}
	return 0;
}

int HotspotHelper::startDnsMasq(MobileHotspotConfiguration &configuration, QProcess *dnsmasqProcess){
	/** -1=fail, 0=ok **/
	/** Credits to Rambo for the basics of the command **/
	qDebug("** Starting dnsmasq **");
	QString cmd("/usr/sbin/dnsmasq");
	cmd += " --no-daemon"; // not running in daemon mode, = do not fork neither run in background
	cmd += " -i "; cmd += configuration.lanInterface; // listen on lan interface
	cmd += " -a "; cmd += configuration.lanNetwork + "1"; // listen on our address
	cmd += " -I lo"; // do not listen on loopback
	cmd += " -z"; // bind only to selected interfaces
	cmd += " --dhcp-range="; cmd += configuration.lanNetwork + "10,"; cmd += configuration.lanNetwork + "200,6h"; // dhcp range
	cmd += " --dhcp-authoritative"; // we are an authoritative dhcp server
	if(configuration.internetEnabled){
		cmd += " --dhcp-option=3,"; cmd += configuration.lanNetwork + "1"; // dhcp option 3 = router
	}
	else{
		cmd += " --port=0"; // disable DNS resolution (if <>0 then it means use this port for DNS)
	}
	cmd += " --dhcp-option=6,"; cmd += configuration.lanNetwork + "1"; // dhcp option 6 = dns server (we give it even if DNS is disabled, as some systems may dislike not knowing any DNS server)
	bool cmdret = CommandHelper::executeCommandAsync(cmd, dnsmasqProcess, MAX_TRIES * 1000);
	if(! cmdret){
		qDebug("FAILURE : Cannot start dnsmasq");
		return -1;
	}
	return 0;
}

int HotspotHelper::stopDnsMasq(MobileHotspotConfiguration &configuration, QProcess *dnsmasqProcess){
	/** -1=fail, 0=ok **/
	qDebug("** Stopping dnsmasq **");
	dnsmasqProcess->terminate();
	return dnsmasqProcess->waitForFinished(MAX_TRIES * 1000) ? 0 : -1;
}

int HotspotHelper::setIPTables(MobileHotspotConfiguration &configuration, int *ipforward){
	/** -1=fail, 0=ok **/
	qDebug("** Setting IPTables **");
	int ret;
	bool cmdret;
	QString cmd("cat /proc/sys/net/ipv4/ip_forward");
	QString res;
	cmdret = CommandHelper::executeCommandWithStdOut(cmd, &ret, &res);
	res = res.left(res.size() - 1); // the last \n of stdout
	if( (!cmdret) || (cmdret && ret != 0))
		res = "0";
	ret = res.toInt(&cmdret);
	if(cmdret)
		*ipforward = ret;
	else
		*ipforward = 0;
	cmd = "/bin/sh -c \"echo "; cmd += configuration.internetEnabled ? "1" : "0";
	cmd += " > /proc/sys/net/ipv4/ip_forward\"";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot modify IP forwarding");
		return -1;
	}
	cmd = "/bin/sh -c \"/usr/sbin/iptables -S > /tmp/iptables.qtmobilehotspot\"";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) )
		qDebug("WARNING : Cannot save iptables");
	cmdret = CommandHelper::executeCommand("/usr/sbin/iptables -F");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -X");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t nat -F");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t nat -X");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t mangle -F");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t mangle -X");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -P FORWARD ACCEPT");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -P INPUT ACCEPT");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -P OUTPUT ACCEPT");
	if(configuration.internetEnabled)
		cmdret = cmdret && CommandHelper::executeCommand(QString("/usr/sbin/iptables -t nat -A POSTROUTING -s ") + configuration.lanNetwork + "0/24 -j MASQUERADE");
	if(! cmdret){
		qDebug("FAILURE : Cannot enable our iptables");
		return -1;
	}
	return 0;
}

int HotspotHelper::unsetIPTables(MobileHotspotConfiguration &configuration, int ipforward){
	/** -1=fail, 0=ok **/
	qDebug("** Unsetting IPTables **");
	int result = 0, ret;
	bool cmdret;
	QString cmd("/bin/sh -c \"echo "); cmd += QString::number(ipforward); cmd += " > /proc/sys/net/ipv4/ip_forward\"";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (cmdret && ret != 0) ){
		qDebug("FAILURE : Cannot restore IP forwarding");
		result = -1;
	}
	cmdret = CommandHelper::executeCommand("/usr/sbin/iptables -F");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t nat -F");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t mangle -F");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -X");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t nat -X");
	cmdret = cmdret && CommandHelper::executeCommand("/usr/sbin/iptables -t mangle -X");
	if(! cmdret){
		qDebug("FAILURE : Cannot flush iptables");
		result = -1;
	}
	QFile file("/tmp/iptables.qtmobilehotspot");
	if(! file.exists()){
		qDebug("FAILURE : iptables save does not exist");
		result = -1;
	}
	else
		if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
			qDebug("FAILURE : Cannot open iptables save file");
			result = -1;
		}
		else{
			QTextStream stream(&file);
			while(! stream.atEnd()){
				cmd = "/usr/sbin/iptables ";
				cmd += stream.readLine();
				cmdret = CommandHelper::executeCommand(cmd, &ret);
				if( (! cmdret) || (cmdret && ret != 0) ){
					qDebug("FAILURE : Invalid command in iptables save");
					result = -1;
				}
			}
			file.close();
		}
	return result;
}
