#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"
#include "pluginconfiguration.h"


PluginConfiguration::PluginConfiguration(){
}

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
	port = helper->load("port", (int)8118).toInt();
	ports = helper->load("ports", "80, 8080").toString();
	QString agent = helper->load("ua", "nochange").toString();
	if(agent == "nochange")
		ua = UA_NOCHANGE;
	else if(agent == "microb")
		ua = UA_MICROB;
	else if(agent == "iphone")
		ua = UA_IPHONE;
	else
		ua = UA_CUSTOM;
	customUA = helper->load("customUA", "").toString();
	defaultActions = helper->load("defaultActions", false).toBool();
	forward = helper->load("forward", false).toBool();
	forwardIP = helper->load("forwardIP", "127.0.0.1").toString();
	forwardPort = helper->load("forwardPort", (int)10000).toInt();
}

void PluginConfiguration::save(){
	// Save the plugin's configuration, please use the helper function save("settingName", value)
	// helper->save("settingName", some_variable);
	helper->save("port", port);
	helper->save("ports", ports);
	if(ua == UA_NOCHANGE)
		helper->save("ua", "nochange");
	else if(ua == UA_MICROB)
		helper->save("ua", "microb");
	else if(ua == UA_IPHONE)
		helper->save("ua", "iphone");
	else
		helper->save("ua", "custom");
	helper->save("customUA", customUA);
	helper->save("defaultActions", defaultActions);
	helper->save("forward", forward);
	helper->save("forwardIP", forwardIP);
	helper->save("forwardPort", forwardPort);
}
