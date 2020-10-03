#include <QtCore/QObject>
#include <QtGui/QMainWindow>
#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QProcess>
#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"
#include "../../qtMobileHotspot/src/mobilehotspotplugin.h"
#include "../../qtMobileHotspot/src/mobilehotspotconfiguration.h"
#include "dnsmasqhelper.h"
#include "pluginconfiguration.h"
#include "plugingui.h"
#include "plugin.h"


Plugin::Plugin(){
	helper = new MobileHotspotPluginHelper(this); // create the helper for this plugin
	conf = new PluginConfiguration(helper); // create the configuration of this plugin
	conf->load(); // load plugin's configuration (see PluginConfiguration::load())
	gui = 0; // no plugin GUI for now
}

Plugin::~Plugin(){
	delete conf; // delete this plugin's configuration
	delete helper; // delete this plugin's helper
}

QString Plugin::name(){
	return "org.dest.qtmh.dnsmasq"; // unique name of the plugin, wont be displayed
}

QString Plugin::displayedName(){
	return tr("DNSMasq"); // displayed name in the plugins control window
}

QString Plugin::displayedAuthor(){
	return "Loic Burtin"; // displayed author name in the plugins control window
}

QString Plugin::displayedContact(){
	return "speedoflight@hotmail.fr"; // displayed contact-info in the plugins control window
}

QString Plugin::displayedDescription(){
	QString desc = tr("The DNSMasq plugin allows you to customize your DNS & DHCP settings.") + "\n";
	desc += tr("For example, it can be handy if you plan to do some DNS-filtering.");
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
	return 50; // order is not important for DNSMasq as it is a service provider
}

QStringList Plugin::provided(){
	// Services provided to the hotspot (PROVIDE_DNSDHCP...), may be empty
	// There must not be 2 plugins enabled which provide the same service
	QStringList result;
	result.append(PROVIDE_DNSDHCP);
	return result;
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
	int ret;
	if(service == PROVIDE_DNSDHCP){
		qDebug("** DNSMasq Plugin : Providing DNS/DHCP **");
		dnsmasqProcess = 0;

		ret = DNSMasqHelper::generateConfigurationFile(configuration, conf);
		if(ret != 0){
			qDebug("FAILURE : Was not able to generate DNSMasq configuration file");
			return false;
		}

		dnsmasqProcess = new QProcess();
		ret = DNSMasqHelper::runDNSMasq(configuration, conf, dnsmasqProcess);
		if(ret != 0){
			qDebug("FAILURE : Was not able to start DNSMasq");
			return false;
		}

		return true;
	}

	return true;
}

bool Plugin::unprovide(QString service, const MobileHotspotConfiguration *configuration, bool successStarting){
	// Stop a service provided to the hotspot, eg : PROVIDE_DNSDHCP
	// Return true if everything is fine, false if failure
	this->successStarting = successStarting; // Save success starting

	// Add your own code here if your plugin is a service provider
	// if(service == PROVIDE_DNSDHCP) ...
	bool result = true;
	int ret;
	if(service == PROVIDE_DNSDHCP){
		qDebug("** DNSMasq Plugin : Unproviding DNS/DHCP **");

		/** Stop DNSMasq **/
		if(dnsmasqProcess != 0){
			ret = DNSMasqHelper::stopDNSMasq(configuration, conf, dnsmasqProcess);
			if(ret != 0){
				qDebug("FAILURE : Was not able to stop DNSMasq");
				result = false;
			}
			delete dnsmasqProcess;
			dnsmasqProcess = 0;
		}

		/** Delete configuration file **/
		ret = DNSMasqHelper::deleteConfigurationFile(configuration, conf);
		if(ret != 0){
			qDebug("FAILURE : Was not able to delete DNSMasq configuration file");
			result = false;
		}

		return result;
	}

	return result;
}

void Plugin::beforeStarting(const MobileHotspotConfiguration *configuration){
	// Run before the startup of the hotspot
	this->running = false; // Hotspot is not running for now
	this->configuration = configuration; // Save configuration
	this->successStarting = false; // Hotspot is not started
	this->successFinishing = false; // Hotspot is not terminated

	// Add your own code here
}

void Plugin::afterStarting(const MobileHotspotConfiguration *configuration, bool successStarting){
	// Run after the startup of the hotspot
	this->running = true; // Hotspot is running
	this->configuration = configuration; // Save configuration
	this->successStarting = successStarting; // Save success starting
	this->successFinishing = false; // Hotspot is not terminated

	// Add your own code here
}

void Plugin::beforeStopping(const MobileHotspotConfiguration *configuration, bool successStarting){
	// Run before the shutdown of the hotspot
	this->running = true; // Hotspot is running
	this->configuration = configuration; // Save configuration
	this->successStarting = successStarting; // Save success starting
	this->successFinishing = false; // Hotspot is not terminated

	// Add your own code here
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


Q_EXPORT_PLUGIN2(qtmhdnsmasq, Plugin); // 1st is target name, 2nd is class name
