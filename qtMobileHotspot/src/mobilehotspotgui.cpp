#include <QtGui/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QObject>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QStringListIterator>
#include <QtGui/QCloseEvent>
#include <QtCore/QProcess>
#include <QtCore/QMap>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtCore/QListIterator>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtCore/QTimer>
#include "apnhelper.h"
#include "mobilehotspotplugin.h"
#include "dbushelper.h"
#include "hotspothelper.h"
#include "pluginscontrolgui.h"
#include "accesspointgui.h"
#include "internetaccessgui.h"
#include "languagegui.h"
#include "mobilehotspotgui.h"


MobileHotspotGUI::MobileHotspotGUI(QWidget *parent)
    : QMainWindow(parent)
{
	qApp->installTranslator(&translator);
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);

	dnsmasqProcess = 0;
	loadPlugins();
	sortPlugins();

	configuration.load();
	if(configuration.language == NO_LANGUAGE_SELECTED)
		configureLanguage();
	else
		configureLanguageFinished(false);

	updateDisplayedConfiguration();
}

MobileHotspotGUI::~MobileHotspotGUI()
{

}

void MobileHotspotGUI::closeEvent(QCloseEvent* event){
	if(ui.btnStart->isEnabled())
		event->accept();
	else{
		if(QMessageBox::question(this, tr("Closing the program"), tr("Hotspot is still active ! Do you want to stop it ?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) == QMessageBox::Yes){
			stop();
			event->accept();
		}
		else
			event->ignore();
	}

}

void MobileHotspotGUI::loadPlugins(){
	plugins.clear();
	sortedPlugins.clear();
	pluginsTranslatorsBaseNames.clear();

	QDir dir("./plugins");
	QStringList list = dir.entryList(QDir::Files | QDir::NoSymLinks);
	QStringListIterator iterator(list);
	while(iterator.hasNext()){
		QString filename = iterator.next();
		QString path = dir.absoluteFilePath(filename);
		QPluginLoader loader(path);
		QObject *loaded = loader.instance();
		if(loaded != 0){
			MobileHotspotPlugin *plugin = qobject_cast<MobileHotspotPlugin*>(loaded);
			if(plugin != 0){
				QString name = plugin->name();
				if(plugins.contains(name))
					qDebug( (QString("WARNING : plugin name ") + name + " is already taken").toAscii().data());
				else{
					pluginsTranslatorsBaseNames.insert(name, QFileInfo(dir, filename).completeBaseName());
					plugins.insert(name, plugin);
					qDebug( (QString("Loaded plugin ") + name).toAscii().data() );
				}
			}
		}
		else
			qDebug(loader.errorString().toAscii().data());
	}
	qDebug( (QString("Loaded ") + QString::number(plugins.size()) + " plugins").toAscii().data() );
}

void MobileHotspotGUI::sortPlugins(){
	sortedPlugins.clear();
	QList<QString> keys = plugins.keys();
	QListIterator<QString> iterator(keys);
	while(iterator.hasNext()){
		QString key = iterator.next();
		int order = plugins.value(key)->order();
		int insertAt = sortedPlugins.size();
		for(int i = 0; i < sortedPlugins.size(); i++){
			QString testedKey = sortedPlugins.at(i);
			int testedOrder = plugins.value(testedKey)->order();
			if(order < testedOrder){
				insertAt = i;
				i = sortedPlugins.size();
			}
		}
		sortedPlugins.insert(insertAt, key);
	}
}

void MobileHotspotGUI::updateServicesPlugins(){
	servicesPlugins.clear();
	QList<MobileHotspotPlugin*> values = plugins.values();
	QListIterator<MobileHotspotPlugin*> iterator(values);
	while(iterator.hasNext()){
		MobileHotspotPlugin *plugin = iterator.next();
		if(plugin->isPluginEnabled()){
			QStringList services = plugin->provided();
			QStringListIterator iterator2(services);
			while(iterator2.hasNext()){
				QString service = iterator2.next();
				if(servicesPlugins.contains(service))
					qDebug("WARNING : Service %s is already provided, cannot be given to plugin %s !", service.toAscii().data(), plugin->name().toAscii().data());
				else{
					qDebug("Plugin %s will provide service %s", plugin->name().toAscii().data(), service.toAscii().data());
					servicesPlugins.insert(service, plugin);
				}
			}
		}
	}
}

void MobileHotspotGUI::configurePlugins(){
	pluginsControlGUI = new PluginsControlGUI(this);
	pluginsControlGUI->setConfiguration(&configuration, ui.btnStop->isEnabled(), &plugins, &sortedPlugins);
	connect(pluginsControlGUI, SIGNAL(windowClosed()),
		this, SLOT(configurePluginsFinished()));
	pluginsControlGUI->show();
}

void MobileHotspotGUI::configurePluginsFinished(){
	// Some code that is run after the closing of the Configure Plugins window
}

void MobileHotspotGUI::configureLanguage(){
	LanguageGUI dlg;
	dlg.setConfiguration(configuration);
	if(dlg.exec() != dlg.Accepted){
		if(configuration.language != NO_LANGUAGE_SELECTED)
			return;
		else
			configuration.language = DEFAULT_LANGUAGE;
	}
	else
		dlg.configuration(&configuration);
	configureLanguageFinished(true);
}

void MobileHotspotGUI::configureLanguageFinished(bool saveConfiguration){
	if(saveConfiguration)
		configuration.save();

	QString lang = configuration.language;
	if(lang != DEFAULT_LANGUAGE)
		translator.load(lang, "./lang", "_", ".qtmh.qm");
	else
		translator.load("", "", "", "");

	QListIterator<MobileHotspotPlugin*> iterator(plugins.values());
	while(iterator.hasNext()){
		MobileHotspotPlugin *plugin = iterator.next();
		QString name = plugin->name();
		QTranslator *trans = plugin->translator();

		if(lang != DEFAULT_LANGUAGE)
			trans->load( (pluginsTranslatorsBaseNames.value(name) + "-") + lang, "./lang", "_", ".qm");
		else
			trans->load("", "", "", "");
	}

	ui.retranslateUi(this);
	updateDisplayedConfiguration();
}

void MobileHotspotGUI::usbWindowsAssistant(){
	QString title = tr("USB For Windows Setup");
	bool failed = false;
	QString text;
	QString usbMAC = configuration.usbMAC;
	QString lanNetwork = configuration.lanNetwork;

	try{
		text = tr("This will help you into installing the USB networking driver into Windows. Please disconnect your N900 from your computer before continuing.");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();

		text = tr("First, ensure that you have copied the windows-usb-driver folder from /opt/qtmobilehotspot/docs (or QtMobileHotspot-Driver when connected in mass-storage mode) to your computer. If this is not the case, stop this assistant, and copy the folder.");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();

		text = tr("We are now going to unload every USB module and load the one for USB networking. Ensure that you are disconnected from your PC and that your N900 isn't in any USB mode (no PC-Suite, no mass-storage).");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();

		HotspotHelper::unloadStandardUSB();
		HotspotHelper::unloadNokiaUSB();
		if(HotspotHelper::checkUSBMode() != 0){
			text = tr("It seems that the USB device is busy. Usually it is due to a USB-mode being active (mass storage / pc-suite). Please stop this mode and try again.");
			QMessageBox::critical(this, title, text);
			throw QString();
		}
		if(HotspotHelper::loadStandardUSB() != 0){
			text = tr("The system failed to load the standard g_ether USB module. It may be due to a missing power-kernel or to a system in an inconsistent state. Please try again later.");
			QMessageBox::critical(this, title, text);
			throw QString();
		}
		if(HotspotHelper::prepareUSBInterface(usbMAC, lanNetwork) != 0){
			text = tr("The system failed to turn on the USB interface. It may be due to a system in an inconsistent state, please try again later.");
			QMessageBox::critical(this, title, text);
			throw QString();
		}

		text = tr("Everything seems to be fine on your N900. You can now connect it to your computer, then continue. Do not select any USB-mode when asked.");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();

		text = tr("Are you running Windows 7 / Windows Vista ?");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes){
			text = tr("The computer will ask you about drivers. Tell it that you want to do an expert install (pick the driver yourself).");
			if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
				throw QString();

			text = tr("Now edit the settings in order to make Windows use your windows-usb-driver folder. Make sure not to forget any useful checkbox.");
			if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
				throw QString();
		}
		else{
			text = tr("The computer will try to install a driver automatically (usually). It will fail, that is normal.");
			if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
				throw QString();

			text = tr("Now go to the device manager (control panel, system, device manager). You should see an unknown device called Linux Ethernet RNDIS. Right-click on it, and choose Update driver.");
			if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
				throw QString();

			text = tr("Choose to pick the driver yourself, and tell the program about your windows-usb-driver folder. Do not miss any important checkbox.");
			if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
				throw QString();
		}

		text = tr("Your computer will now install the USB networking driver. If everything goes fine, congratulations, you have a new network device available in the devices-list.");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();

		text = tr("You are now ready to use the program in USB mode with your Windows. Just remember to never use pc-suite/mass-storage modes when you enable USB networking.");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();

		text = tr("Windows should not need more configuration in order to be used with qtmobilehotspot. Simply run qtmobilehotspot, start in USB mode, plug your N900 into your PC, and enjoy.");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();

		text = tr("Please note that if you are already using MADDE on your computer, you may have to reset your new network card settings to automatic.");
		if(QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
			throw QString();
	}
	catch(QString fail){
		qDebug("WARNING : Interrupted Windows USB Assistant");
		failed = true;
	}

	text = tr("Please disconnect your N900 from your computer. We are going to unload the USB module and load the normal Nokia one. Do not choose any USB mode if you plan to use qtmobilehotspot.");
	if(! failed)
		QMessageBox::information(this, title, text);
	HotspotHelper::unprepareUSBInterface();
	HotspotHelper::unloadStandardUSB();
	HotspotHelper::loadNokiaUSB();
}

void MobileHotspotGUI::updateDisplayedConfiguration(){
	ui.lstAccessPoint->clear();

	QString s = configuration.hotspotName + " (";
	s += configuration.encryptionType == configuration.ENCRYPTION_WEP ? tr("WEP") : tr("Open");
	s += ")";
	ui.lstAccessPoint->addItem(s);
	if(configuration.wlanTypeMAC == MAC_CUSTOM){
		s = tr("Custom MAC");
		s += " (";
		s += configuration.wlanCustomMAC + ")";
		ui.lstAccessPoint->addItem(s);
	}
	else if(configuration.wlanTypeMAC == MAC_RANDOM)
		ui.lstAccessPoint->addItem(tr("Random MAC"));
	if(configuration.cycleWlanDriver)
		ui.lstAccessPoint->addItem(tr("Restart Wifi driver"));

	ui.lstInternetAccess->clear();
	s = configuration.internetAPName;
	if(s != "" && configuration.internetInterface() == configuration.INTERFACE_GPRS){
		if(configuration.enforcement2g3g != ENFORCEMENT_2G3G_IGNORE){
			s += " (";
			if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_2G)
				s += tr("2G");
			else if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_3G)
				s += tr("3G");
			else
				s += tr("2G/3G");
			s += ")";
		}
	}
	ui.lstInternetAccess->addItem(s);
	ui.lstInternetAccess->addItem(configuration.internetEnabled ? tr("With internet") : tr("Without internet"));
	ui.lstInternetAccess->addItem(configuration.lanNetwork + "0");

	ui.lstInterfaces->clear();
	ui.lstInterfaces->addItem(tr("Wifi"), configuration.INTERFACE_WLAN);
	ui.lstInterfaces->addItem(tr("USB"), configuration.INTERFACE_USB);
	ui.lstInterfaces->setCurrentIndex(configuration.lanInterface == configuration.INTERFACE_WLAN ? 0 : 1);
}

void MobileHotspotGUI::configureInternetAccess(){
	internetAccessGUI = new InternetAccessGUI(this);
	internetAccessGUI->setConfiguration(configuration);
	connect(internetAccessGUI, SIGNAL(windowClosed()),
		this, SLOT(configureInternetAccessFinished()));
	internetAccessGUI->show();
}

void MobileHotspotGUI::configureInternetAccessFinished(){
	internetAccessGUI->configuration(&configuration);
	configuration.save();
	updateDisplayedConfiguration();
}

void MobileHotspotGUI::configureAccessPoint(){
	accessPointGUI = new AccessPointGUI(this);
	accessPointGUI->setConfiguration(configuration);
	connect(accessPointGUI, SIGNAL(windowClosed()),
		this, SLOT(configureAccessPointFinished()));
	accessPointGUI->show();
}

void MobileHotspotGUI::configureAccessPointFinished(){
	accessPointGUI->configuration(&configuration);
	configuration.save();
	updateDisplayedConfiguration();
}

void MobileHotspotGUI::configureInterface(int index){
	configuration.lanInterface = ui.lstInterfaces->itemData(index).toString();
	if(configuration.lanInterface == configuration.INTERFACE_WLAN && configuration.internetAPType != GCONF_APN_TYPE_GPRS){
		configuration.internetAPID = "";
		configuration.internetAPType = "";
		configuration.internetAPName = "";
	}
	configuration.save();
	updateDisplayedConfiguration();
}

void MobileHotspotGUI::start(bool notification){
	int ret;

	/** Find previous enforcement **/
	if(configuration.internetEnabled && configuration.enforcement2g3g != ENFORCEMENT_2G3G_IGNORE){
		ret = DBUSHelper::internetEnforcement2G3G(&previousEnforcement);
		if(ret != 0){
			qDebug("WARNING : Cannot get the current level of 2G/3G enforcement, using default value DOUBLE");
			previousEnforcement = DBUS_2G3G_DOUBLE;
		}
	}

	/** Find previous connection **/
	previousConnectionID = "";
	previousConnectionType = "";
	waitingForPreviousConnection = true;
	previousConnectionEnableSignal();
	previousConnectionRequest();

	/** Start the hotspot a bit later (hopefully the previous connection will be known) **/
	showNotifications = notification;
	QTimer::singleShot(600, this, SLOT(startProcedure()));
}

void MobileHotspotGUI::startProcedure(){
	/* Credits to Rambo for the procedure */
	bool success = true;
	int ret;
	qDebug("*** STARTING PROCESS ***");

	/** Disable previous connection search **/
	waitingForPreviousConnection = false;
	previousConnectionDisableSignal();

	/** Obvious sanity checks **/
	if(configuration.lanInterface == configuration.INTERFACE_WLAN && configuration.internetInterface() == configuration.INTERFACE_WLAN){
		/** Both LAN and INET are on WLAN, that is not possible **/
		QString fail = tr("Error : LAN and Internet are both using the Wifi interface, please use a LAN on USB");
		QMessageBox::critical(this, tr("Cannot start"), fail);
		return;
	}

	/** Save scan interval **/
	scanInterval = APNHelper::scanInterval();

	/** Find plugin-provided services **/
	updateServicesPlugins();

	/** Notify plugins **/
	QListIterator<QString> pluginsIterator = QListIterator<QString>(sortedPlugins);
	while(pluginsIterator.hasNext()){
		MobileHotspotPlugin *plugin = plugins.value(pluginsIterator.next());
		if(plugin->isPluginEnabled()){
			qDebug( (QString("Plugin beforeStarting : ") + plugin->name()).toAscii().data() );
			qApp->installTranslator(plugin->translator());
				plugin->beforeStarting(&configuration);
			qApp->removeTranslator(plugin->translator());
		}
	}

	try{
		/** System Modules **/
		if(servicesPlugins.contains(PROVIDE_SYSTEMMODULES)){
			/** Plugin-provided service **/
			MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_SYSTEMMODULES);
			if(! plugin->provide(PROVIDE_SYSTEMMODULES, &configuration))
				throw tr("Error while loading system modules using a plugin");
		}
		else{
			/** Load system modules if necessary **/
			alreadyLoadedModules.clear();
			ret = HotspotHelper::loadSystemModules(&alreadyLoadedModules);
			if(ret != 0)
				throw tr("Error while loading system modules, are you running the power kernel ?");
		}

		/** Internet connection handling **/
		if(configuration.internetEnabled){
			qDebug("Previous internet connection was %s (%s), next one is %s (%s)", previousConnectionID.toAscii().data(), previousConnectionType.toAscii().data(), configuration.internetAPID.toAscii().data(), configuration.internetAPType.toAscii().data());
			if(previousConnectionType == configuration.internetAPType && previousConnectionID == configuration.internetAPID && configuration.internetInterface() != configuration.INTERFACE_WLAN)
				/** Nothing to do to the internet connection **/
				/** In case of USB-Lan and Wifi-Inet we have to disconnect & reconnect to prevent route disappearing problem **/
				qDebug("** Internet connection is already good, skipping this step **");
			else{
				/** Disable current internet connection **/
				ret = HotspotHelper::disableInternetConnection(configuration);
				if(ret != 0)
					throw tr("Error while disconnecting previous internet connection");

				/** Connect to the chosen APN **/
				ret = HotspotHelper::enableInternetConnection(configuration);
				if(ret != 0)
					throw tr("Error while connecting to the APN, you way need to select it again in the Networking panel");
			}
		}
		else if(configuration.lanInterface == configuration.INTERFACE_WLAN && previousConnectionType.indexOf("WLAN") >= 0){
			/** WLAN must be disabled in order to get the adhoc WLAN working **/
			ret = HotspotHelper::disableInternetConnection(configuration);
			if(ret != 0)
				throw tr("Error while disconnecting previous internet connection");
		}
		else
			qDebug("** User requested no internet connection, ignoring it then **");

		/** Disable network automatic search **/
		APNHelper::setScanInterval(0);

		/** USB vs Wifi **/
		if(configuration.lanInterface == configuration.INTERFACE_USB){

			/** USB Module **/
			if(servicesPlugins.contains(PROVIDE_USBMODULE)){
				/** Plugin-provided service **/
				MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_USBMODULE);
				if(! plugin->provide(PROVIDE_USBMODULE, &configuration))
					throw tr("Error while loading USB module using a plugin");
			}
			else{
				/** Unload all USB modules - May fail for some, but thats normal **/
				HotspotHelper::unloadNokiaUSB();
				HotspotHelper::unloadStandardUSB();

				/** Check USB active module **/
				ret = HotspotHelper::checkUSBMode();
				if(ret != 0)
					throw tr("Error : USB shouldn't be in any mode (no storage mode, no pc-suite mode)");

				/** Load standard USB module **/
				ret = HotspotHelper::loadStandardUSB();
				if(ret != 0)
					throw tr("Error while loading standard g_ether driver");
			}

			/** USB Interface **/
			if(servicesPlugins.contains(PROVIDE_USBINTERFACE)){
				/** Plugin-provided service **/
				MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_USBINTERFACE);
				if(! plugin->provide(PROVIDE_USBINTERFACE, &configuration))
					throw tr("Error while preparing the USB interface using a plugin");
			}
			else{
				/** Prepare USB interface **/
				ret = HotspotHelper::prepareUSBInterface(configuration.usbMAC, configuration.lanNetwork);
				if(ret != 0)
					throw tr("Error while preparing the USB interface");
			}

		}
		else{

			/** Wifi Interface **/
			if(servicesPlugins.contains(PROVIDE_WLANINTERFACE)){
				/** Plugin-provided service **/
				MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_WLANINTERFACE);
				if(! plugin->provide(PROVIDE_WLANINTERFACE, &configuration))
					throw tr("Error while preparing the Wifi interface using a plugin");
			}
			else{
				/** Prepare Wifi interface **/
				ret = HotspotHelper::prepareWifiInterface(configuration, &previousWlanMAC);
				if(ret != 0)
					throw tr("Error while preparing the Wifi interface");
			}

		}

		/** DNS-DHCP **/
		if(servicesPlugins.contains(PROVIDE_DNSDHCP)){
			/** Plugin-provided service **/
			MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_DNSDHCP);
			if(! plugin->provide(PROVIDE_DNSDHCP, &configuration))
				throw tr("Error while starting DNS/DHCP server using a plugin");
		}
		else{
			/** Start dnsmasq **/
			dnsmasqProcess = new QProcess();
			ret = HotspotHelper::startDnsMasq(configuration, dnsmasqProcess);
			if(ret != 0)
				throw tr("Error while starting dnsmasq");
		}

		/** Iptables */
		if(servicesPlugins.contains(PROVIDE_IPTABLES)){
			/** Plugin-provided service **/
			MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_IPTABLES);
			if(! plugin->provide(PROVIDE_IPTABLES, &configuration))
				throw tr("Error while setting iptables using a plugin");
		}
		else{
			/** Set iptables **/
			ret = HotspotHelper::setIPTables(configuration, &ipforward);
			if(ret != 0)
				throw tr("Error while setting iptables");
		}

		// There :) Finished
	}
	catch(QString error){
		qDebug(error.toAscii().data());
		success= false;
		if(showNotifications)
			QMessageBox::critical(this, tr("Failed to start the hotspot"), error);
	}

	/** Notify plugins **/
	successStarting = success;
	pluginsIterator = QListIterator<QString>(sortedPlugins);
	while(pluginsIterator.hasNext()){
		MobileHotspotPlugin *plugin = plugins.value(pluginsIterator.next());
		if(plugin->isPluginEnabled()){
			qDebug( (QString("Plugin afterStarting : ") + plugin->name()).toAscii().data() );
			qApp->installTranslator(plugin->translator());
				plugin->afterStarting(&configuration, successStarting);
			qApp->removeTranslator(plugin->translator());
		}
	}

	/** Enforcement 2G/3G **/
	if(configuration.internetEnabled && configuration.enforcement2g3g != ENFORCEMENT_2G3G_IGNORE && configuration.internetInterface() == configuration.INTERFACE_GPRS){
		qDebug("Changing 2g/3g enforcement");
		ret = 0;
		if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_2G && previousEnforcement != DBUS_2G3G_2G)
			ret = DBUSHelper::setInternetEnforcement2G3G(DBUS_2G3G_2G);
		else if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_3G && previousEnforcement != DBUS_2G3G_3G)
			ret = DBUSHelper::setInternetEnforcement2G3G(DBUS_2G3G_3G);
		else if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_DOUBLE && previousEnforcement != DBUS_2G3G_DOUBLE)
			ret = DBUSHelper::setInternetEnforcement2G3G(DBUS_2G3G_DOUBLE);
		if(ret != 0)
			qDebug("WARNING : Cannot change 2g/3g enforcement");
	}

	/** GUI **/
	if(success){
		if(showNotifications)
			DBUSHelper::notificationSimple(tr("Hotspot started successfully"));
		ui.btnAccessPoint->setEnabled(false);
		ui.btnInternetAccess->setEnabled(false);
		ui.lstInterfaces->setEnabled(false);
		ui.btnStart->setEnabled(false);
		ui.btnStop->setEnabled(true);
		ui.actionUSBAssistant->setEnabled(false);
		qDebug("*** STARTING PROCESS TERMINATED SUCCESSFULLY ***");
	}
	else{
		qDebug("*** STARTING PROCESS FAILED ***");
		stop(false);
	}
}

void MobileHotspotGUI::stop(bool notification){
	/** Nothing to do here, its simply for symetry with start() **/
	showNotifications = notification;
	stopProcedure();
}

void MobileHotspotGUI::stopProcedure(){
	/* Credits to Rambo for the procedure */
	bool success = true;
	int ret;
	qDebug("*** STOPPING PROCESS ***");

	/** Find plugin-provided services **/
	updateServicesPlugins();

	/** Notify plugins **/
	QListIterator<QString> pluginsIterator = QListIterator<QString>(sortedPlugins);
	pluginsIterator.toBack();
	while(pluginsIterator.hasPrevious()){
		MobileHotspotPlugin *plugin = plugins.value(pluginsIterator.previous());
		if(plugin->isPluginEnabled()){
			qDebug( (QString("Plugin beforeStopping : ") + plugin->name()).toAscii().data() );
			qApp->installTranslator(plugin->translator());
				plugin->beforeStopping(&configuration, successStarting);
			qApp->removeTranslator(plugin->translator());
		}
	}

	/** Iptables */
	if(servicesPlugins.contains(PROVIDE_IPTABLES)){
		/** Plugin-provided service **/
		MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_IPTABLES);
		if(! plugin->unprovide(PROVIDE_IPTABLES, &configuration, successStarting)){
			qDebug("Error while unsetting iptables using plugin : %s", plugin->name().toAscii().data());
			success = false;
		}
	}
	else{
		/** Unset iptables **/
		ret = HotspotHelper::unsetIPTables(configuration, ipforward);
		if(ret != 0){
			qDebug("Error while unsetting iptables");
			success = false;
		}
	}

	/** DNS-DHCP **/
	if(servicesPlugins.contains(PROVIDE_DNSDHCP)){
		/** Plugin-provided service **/
		MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_DNSDHCP);
		if(! plugin->unprovide(PROVIDE_DNSDHCP, &configuration, successStarting)){
			qDebug("Error while stopping DNS/DHCP server using plugin : %s", plugin->name().toAscii().data());
			success = false;
		}
	}
	else{
		/** Stop dnsmasq **/
		if(dnsmasqProcess != 0){
			ret = HotspotHelper::stopDnsMasq(configuration, dnsmasqProcess);
			if(ret != 0){
				qDebug("Error while stopping dnsmasq");
				success = false;
			}
			delete dnsmasqProcess;
			dnsmasqProcess = 0;
		}
	}

	/** USB vs Wifi **/
	if(configuration.lanInterface == configuration.INTERFACE_USB){

		/** USB Interface **/
		if(servicesPlugins.contains(PROVIDE_USBINTERFACE)){
			/** Plugin-provided service **/
			MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_USBINTERFACE);
			if(! plugin->unprovide(PROVIDE_USBINTERFACE, &configuration, successStarting)){
				qDebug("Error while unpreparing USB interface using plugin : %s", plugin->name().toAscii().data());
				success = false;
			}
		}
		else{
			/** Unprepare USB interface **/
			ret = HotspotHelper::unprepareUSBInterface();
			if(ret != 0){
				qDebug("Error while unpreparing USB interface");
				success = false;
			}
		}

		/** USB Module **/
		if(servicesPlugins.contains(PROVIDE_USBMODULE)){
			/** Plugin-provided service **/
			MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_USBMODULE);
			if(! plugin->unprovide(PROVIDE_USBMODULE, &configuration, successStarting)){
				qDebug("Error while unloading USB module using plugin : %s", plugin->name().toAscii().data());
				success = false;
			}
		}
		else{
			/** Unload all USB modules - May fail for some, but thats normal **/
			HotspotHelper::unloadNokiaUSB();
			HotspotHelper::unloadStandardUSB();

			/** Check USB active module **/
			ret = HotspotHelper::checkUSBMode();
			if(ret != 0){
				qDebug("Error while checking usb active modules (should be none, but thats not the case)");
				success = false;
			}

			/** Load Nokias USB module **/
			ret = HotspotHelper::loadNokiaUSB();
			if(ret != 0){
				qDebug("Error while loading Nokia USB module");
				success = false;
			}
		}

	}
	else{

		/** Wifi Interface **/
		if(servicesPlugins.contains(PROVIDE_WLANINTERFACE)){
			/** Plugin-provided service **/
			MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_WLANINTERFACE);
			if(! plugin->unprovide(PROVIDE_WLANINTERFACE, &configuration, successStarting)){
				qDebug("Error while unpreparing Wifi interface using plugin : %s", plugin->name().toAscii().data());
				success = false;
			}
		}
		else{
			/** Unprepare Wifi interface **/
			ret = HotspotHelper::unprepareWifiInterface(configuration, previousWlanMAC);
			if(ret != 0){
				qDebug("Error while unpreparing Wifi interface");
				success = false;
			}
		}

	}

	/** Restore scan interval **/
	APNHelper::setScanInterval(scanInterval);

	/** Internet connection handling **/
	if(configuration.internetEnabled){

		if(previousConnectionType == configuration.internetAPType && previousConnectionID == configuration.internetAPID && configuration.internetInterface() != configuration.INTERFACE_WLAN)
			/** Current connection == previous connection **/
			/** In case of USB-Lan and Wifi-Inet we have to disconnect, change powersave & reconnect **/
			qDebug("** Previous connection was the same, nothing to do to the internet connection **");
		else{
			/** Disable current internet connection **/
			ret = HotspotHelper::disableInternetConnection(configuration);
			if(ret != 0){
				qDebug("Error while disconnecting internet connection");
				success = false;
			}

			if(previousConnectionID != ""){
				/** Restore previous internet connection **/
				qDebug("** Restoring previous internet connection **");
				ret = DBUSHelper::internetConnect(previousConnectionID);
				if(ret != 0){
					qDebug("Error while reconnecting previous internet connection");
					success = false;
				}
			}
			else
				qDebug("** There was no previous connection, so no reconnection to do **");
		}
	}
	else if(configuration.lanInterface == configuration.INTERFACE_WLAN && previousConnectionType.indexOf("WLAN") >= 0){
		/** We need to restore disabled WLAN **/
		ret = DBUSHelper::internetConnect(previousConnectionID);
		if(ret != 0){
			qDebug("Error while reconnecting previous internet connection");
			success = false;
		}
	}
	else
		qDebug("** User requested no internet connection, ignoring it then **");


	/** System Modules **/
	if(servicesPlugins.contains(PROVIDE_SYSTEMMODULES)){
		/** Plugin-provided service **/
		MobileHotspotPlugin *plugin = servicesPlugins.value(PROVIDE_SYSTEMMODULES);
		if(! plugin->unprovide(PROVIDE_SYSTEMMODULES, &configuration, successStarting)){
			qDebug("Error while unloading system modules using plugin : %s", plugin->name().toAscii().data());
			success = false;
		}
	}
	else{
		/** Unload system modules if we loaded them ourselves **/
		ret = HotspotHelper::unloadSystemModules(alreadyLoadedModules);
		if(ret != 0){
			qDebug("Error while unloading system modules");
			// not that critical, no need for a message about this
		}
	}


	// There :) Finished

	/** Notify plugins **/
	pluginsIterator = QListIterator<QString>(sortedPlugins);
	pluginsIterator.toBack();
	while(pluginsIterator.hasPrevious()){
		MobileHotspotPlugin *plugin = plugins.value(pluginsIterator.previous());
		if(plugin->isPluginEnabled()){
			qDebug( (QString("Plugin afterStopping : ") + plugin->name()).toAscii().data() );
			qApp->installTranslator(plugin->translator());
				plugin->afterStopping(&configuration, successStarting, success);
			qApp->removeTranslator(plugin->translator());
		}
	}

	/** Enforcement 2G/3G **/
	if(configuration.internetEnabled && configuration.enforcement2g3g != ENFORCEMENT_2G3G_IGNORE && configuration.internetInterface() == configuration.INTERFACE_GPRS){
		qDebug("Restoring original 2g/3g enforcement");
		ret = DBUSHelper::setInternetEnforcement2G3G(previousEnforcement);
		if(ret != 0)
			qDebug("WARNING : Cannot restore previous 2g/3g enforcement");
	}

	/** GUI **/
	if(showNotifications)
		DBUSHelper::notificationSimple(success ? tr("Hotspot stopped successfully") : tr("Hotspot stopped with errors"));
	ui.btnAccessPoint->setEnabled(true);
	ui.btnInternetAccess->setEnabled(true);
	ui.lstInterfaces->setEnabled(true);
	ui.btnStart->setEnabled(true);
	ui.btnStop->setEnabled(false);
	ui.actionUSBAssistant->setEnabled(true);

	updateDisplayedConfiguration();
	qDebug("*** STOPPING PROCESS FINISHED ***");
}

void MobileHotspotGUI::previousConnectionSignal(const QDBusMessage &message){
	if(! waitingForPreviousConnection)
		return;
	QList<QVariant> args = message.arguments();
	if(args.size() != 8)
		return;
	QString errorMessage = args.at(6).toString();
	if(errorMessage != ""){
		qDebug((QString("Error while testing previous connection : ") + errorMessage).toAscii().data());
		return;
	}
	previousConnectionID = QString(args.at(5).toByteArray());
	previousConnectionType = args.at(3).toString();
}

void MobileHotspotGUI::previousConnectionEnableSignal(){
	QDBusConnection dbus = QDBusConnection::systemBus();
	dbus.connect("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "state_sig", this, SLOT(previousConnectionSignal(const QDBusMessage&)));
}

void MobileHotspotGUI::previousConnectionDisableSignal(){
	QDBusConnection dbus = QDBusConnection::systemBus();
	dbus.disconnect("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "state_sig", this, SLOT(previousConnectionSignal(const QDBusMessage&)));
}

void MobileHotspotGUI::previousConnectionRequest(){
	QDBusConnection dbus = QDBusConnection::systemBus();
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "state_req");
	dbus.send(msg);
}

