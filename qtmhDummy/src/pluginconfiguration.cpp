#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"
#include "pluginconfiguration.h"


PluginConfiguration::PluginConfiguration(MobileHotspotPluginHelper *helper){
	this->helper = helper; // register our helper, it is useful for loading/saving and such
}

PluginConfiguration::~PluginConfiguration(){
	// helper deletion is handled by plugin itself, no need to destroy it here
}

void PluginConfiguration::reset(){
	// Reset the plugin's configuration, by deleting the saved one and calling load()
	helper->deleteConfiguration(); // Delete saved configuration

	// You may want to do some things here before calling load, such as freeing memory
	// Type here your customized code
	// delete myVariable;

	load(); // Load configuration (the default one, as none is saved)
}

void PluginConfiguration::load(){
	// Load the plugin's configuration, please use the helper function load("settingName", "defaultValue")
	// some_variable = helper->load("settingName", "default value").toString();
}

void PluginConfiguration::save(){
	// Save the plugin's configuration, please use the helper function save("settingName", value)
	// helper->save("settingName", some_variable);
}
