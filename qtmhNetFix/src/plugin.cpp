#include <QtCore/QObject>
#include <QtGui/QMainWindow>
#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QTimer>
#include <QtCore/QProcess>
#include "../../qtMobileHotspot/src/apnhelper.h"
#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"
#include "../../qtMobileHotspot/src/mobilehotspotplugin.h"
#include "../../qtMobileHotspot/src/mobilehotspotconfiguration.h"
#include "../../qtMobileHotspot/src/commandhelper.h"
#include "pluginconfiguration.h"
#include "plugingui.h"
#include "plugin.h"


Plugin::Plugin(){
	helper = new MobileHotspotPluginHelper(this); // create the helper for this plugin
	conf = new PluginConfiguration(helper); // create the configuration of this plugin
	conf->load(); // load plugin's configuration (see PluginConfiguration::load())
	gui = 0; // no plugin GUI for now
	timedOperationsEnabled = false;
}

Plugin::~Plugin(){
	delete conf; // delete this plugin's configuration
	delete helper; // delete this plugin's helper
	timedOperationsEnabled = false;
}

QString Plugin::name(){
	return "org.dest.qtmh.netfix"; // unique name of the plugin, wont be displayed
}

QString Plugin::displayedName(){
	return tr("Net Fixer"); // displayed name in the plugins control window
}

QString Plugin::displayedAuthor(){
	return "Loic Burtin"; // displayed author name in the plugins control window
}

QString Plugin::displayedContact(){
	return "speedoflight@hotmail.fr"; // displayed contact-info in the plugins control window
}

QString Plugin::displayedDescription(){
	QString desc = tr("This plugin embeds several functions for fixing general hotspot slowdowns.") + "\n";
	desc += tr("You may try them and see if they help.");
	return desc; // displayed description in the plugins control window
}

QIcon Plugin::displayedIcon(){
	return QIcon(); // displayed icon in the plugins control window
}

bool Plugin::allowEnableDisableStopped(){
	return true; // allow (or not) the user to enable & disable the plugin while the hotspot is stopped
}

bool Plugin::allowEnableDisableRunning(){
	return false; // allow (or not) the user to enable & disable the plugin while the hotspot is running
}

bool Plugin::allowConfigureStopped(){
	return true; // allow (or not) the user to configure the plugin while hotspot is stopped
}

bool Plugin::allowConfigureRunning(){
	return false; // allow (or not) the user to configure the plugin while hotspot is running
}

int Plugin::order(){
	// Plugins call ordering
	// Range should be 0..100 (included)
	// Startup calls are done in natural order (0, 1, ..., 100)
	// Shutdown calls are done in reverse order (100, 99, ..., 0)
	// Ranges 0..20 and 80..100 (included) should be reserved to 'official' plugins.
	return 20; // not important, should be run before 21..79 plugins thought
}

QStringList Plugin::provided(){
	// Services provided to the hotspot (PROVIDE_DNSDHCP...), may be empty
	// There must not be 2 plugins enabled which provide the same service
	return QStringList();
}

bool Plugin::isPluginEnabled(){
	// Return the enable status of the plugin
	// Should work for 99% of the plugins, feel free to tweak it if you want
	return helper->isPluginEnabled(); // return true if the plugin is enabled, false if it is not
}

void Plugin::setPluginEnabled(bool enabled, const MobileHotspotConfiguration *configuration, bool running){
	// Set the enable status of the plugin
	// Should work for all plugins whose enable status cannot be changed while running
	this->running = running; // Save running
	this->configuration = configuration; // Save configuration
	helper->setPluginEnabled(enabled); // true if the plugin is enabled, false if it is not

	// If your plugin can be enabled/disabled while running, you might want to add some code in this method (stop a process...)
	// eg if(running){ do_some_things(configuration); }
}

void Plugin::configure(const QMainWindow *parentWindow, const MobileHotspotConfiguration *configuration, bool running, bool reset){
	// User clicked the Configure or the Reset button in the control window
	this->running = running; // Save running
	this->configuration = configuration; // Save configuration
	helper->installTranslator(); // Enable our translations - Usually QTMH does it itself, but as we are using our own windows, it is our job this time
	if(reset)
		conf->reset(); // Reset the configuration before showing it on the GUI
	QMainWindow *window = const_cast<QMainWindow*>(parentWindow); // Unconst the parentWindow
	gui = new PluginGUI(window); // create the GUI
	gui->setConfiguration(*conf); // load plugin configuration into the GUI
	connect(gui, SIGNAL(windowClosed()),
			this, SLOT(configureFinished())); // connect our callback at GUI closing
	gui->show(); // show the GUI
	// You should not have to add anything here, as it will be executed while GUI is shown or after configuration reset
	// If you want to run some code, see configureFinished()
}

void Plugin::configureFinished(){
	// Configuration GUI has been closed
	gui->configuration(conf); // Get plugin configuration from the GUI
	conf->save(); // Save plugin's configuration (see PluginConfiguration::save())
	helper->removeTranslator(); // Disable plugin translations, usually QTMH does it but this time its our job - see configure()

	// You may want to add here some things to do, usually there are some if configuration has been changed while the hotspot was running
	// eg if(this->running){ do_some_things(this->configuration); }
}

bool Plugin::provide(QString service, const MobileHotspotConfiguration *configuration){
	// Start a service provided to the hotspot, eg : PROVIDE_DNSDHCP
	// Return true if everything is fine, false if failure
	// Add your own code here if your plugin is a service provider
	// if(service == PROVIDE_DNSDHCP) ...
	return true;
}

bool Plugin::unprovide(QString service, const MobileHotspotConfiguration *configuration, bool successStarting){
	// Stop a service provided to the hotspot, eg : PROVIDE_DNSDHCP
	// Return true if everything is fine, false if failure
	this->successStarting = successStarting; // Save success starting

	// Add your own code here if your plugin is a service provider
	// if(service == PROVIDE_DNSDHCP) ...
	return true;
}

void Plugin::beforeStarting(const MobileHotspotConfiguration *configuration){
	// Run before the startup of the hotspot
	this->running = false; // Hotspot is not running for now
	this->configuration = configuration; // Save configuration
	this->successStarting = false; // Hotspot is not started
	this->successFinishing = false; // Hotspot is not terminated

	// Add your own code here
	int ret;
	if(configuration->internetEnabled){
		if(configuration->internetInterface() == configuration->INTERFACE_WLAN){
			if(conf->disablePowerSaving){
				previousPowerSave = APNHelper::apnPowerSave(configuration->internetAPID);
				APNHelper::setAPNPowerSave(configuration->internetAPID, GCONF_APN_POWER_NOECO);
			}
		}
	}
}

void Plugin::afterStarting(const MobileHotspotConfiguration *configuration, bool successStarting){
	// Run after the startup of the hotspot
	this->running = true; // Hotspot is running
	this->configuration = configuration; // Save configuration
	this->successStarting = successStarting; // Save success starting
	this->successFinishing = false; // Hotspot is not terminated

	// Add your own code here
	if(configuration->internetEnabled && configuration->internetInterface() == configuration->INTERFACE_WLAN && conf->disablePowerSaving){
		APNHelper::setAPNPowerSave(configuration->internetAPID, previousPowerSave);
		setPowerManagement(MobileHotspotConfiguration::INTERFACE_WLAN, false);
	} // MUST BE BEFORE THE SUCCESSSTARTING CHECK, WE MUST RESTORE POWERMANAGEMENT EVEN IF SUCCESSSTARTING == FALSE

	if(! successStarting)
		return;

	if(conf->wlanCheck && configuration->lanInterface == configuration->INTERFACE_WLAN)
		QTimer::singleShot(TIME_WLANCHECK, this, SLOT(mustCheckWLAN()));

	timedOperationsEnabled = true;
}

void Plugin::beforeStopping(const MobileHotspotConfiguration *configuration, bool successStarting){
	// Run before the shutdown of the hotspot
	this->running = true; // Hotspot is running
	this->configuration = configuration; // Save configuration
	this->successStarting = successStarting; // Save success starting
	this->successFinishing = false; // Hotspot is not terminated

	// Add your own code here
	if(! successStarting)
		return;
	timedOperationsEnabled = false;
}

void Plugin::afterStopping(const MobileHotspotConfiguration *configuration, bool successStarting, bool successFinishing){
	// Run after the shutdown of the hotspot
	this->running = false; // Hotspot is running
	this->configuration = configuration; // Save configuration
	this->successStarting = successStarting; // Save success starting
	this->successFinishing = successFinishing; // Save success finishing

	// Add your own code here
}

QTranslator* Plugin::translator(){
	// Translations need
	// Should be kept as is, even if there are no translations
	return &translater;
}

int Plugin::setPowerManagement(QString interface, bool management){
	/* -1=fail, -2=timeout, 0=ok */
	int ret;
	bool cmdret;
	QString cmd("/sbin/iwconfig "); cmd += interface;
	cmd += management ? " power on" : " power off";
	cmdret = CommandHelper::executeCommand(cmd, &ret);
	if( (! cmdret) || (ret != 0) ){
		qDebug("WARNING : NetFix :: Cannot modify power management");
		return -1;
	}
	return 0;
}

void Plugin::mustCheckWLAN(){
	if(! timedOperationsEnabled)
		return;
	qDebug("NetFix :: Checking WLAN status");
	int ret;
	bool success;
	QString res;
	success = CommandHelper::executeCommandWithStdOut("route", &ret, &res);
	if( (! success) || (ret != 0) )
		qDebug("WARNING : NetFix :: Cannot check WLAN status (route didn't succeed)");
	else{
		if(res.indexOf(MobileHotspotConfiguration::INTERFACE_WLAN) < 0){
			qDebug("NetFix :: WLAN seems to be down (no result in routing table), trying to switch it up");
			CommandHelper::executeCommandOld(QString("ifconfig ") + MobileHotspotConfiguration::INTERFACE_WLAN + " up", &ret);
			if(ret != 0)
				qDebug("WARNING : NetFix :: Cannot switch up WLAN");
			else
				qDebug("NetFix :: Switched up WLAN");
			QString command = QString("iwconfig ") + MobileHotspotConfiguration::INTERFACE_WLAN + " essid \"";
			command += configuration->hotspotName + "\"";
			CommandHelper::executeCommandOld(command, &ret);
			if(ret != 0)
				qDebug("WARNING : NetFix :: Cannot re-enter the WLAN ESSID");
			else
				qDebug("NetFix :: Re-entered the WLAN ESSID");
		}
	}
	QTimer::singleShot(TIME_WLANCHECK, this, SLOT(mustCheckWLAN()));
}


Q_EXPORT_PLUGIN2(qtmhnetfix, Plugin); // 1st is target name, 2nd is class name
