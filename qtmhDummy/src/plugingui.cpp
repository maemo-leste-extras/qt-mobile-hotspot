#include <QtGui/QCloseEvent>
#include <QtGui/QMenuBar>
#include "plugingui.h"
#include "pluginconfiguration.h"


PluginGUI::PluginGUI(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);
}

PluginGUI::~PluginGUI()
{

}

void PluginGUI::closeEvent(QCloseEvent* event){
	// Signal that the GUI is closed
	if(menuBar() != 0)
		menuBar()->clear(); // Solves a bug where menubar in child gui is kept in parent gui
	emit windowClosed();
	event->accept();
}

void PluginGUI::configuration(PluginConfiguration *conf){
	// Store entered configuration into the configuration object
	// conf->some_variable = ui.some_component->someGetter();
}

void PluginGUI::setConfiguration(PluginConfiguration &conf){
	// Display configuration from the configuration object
	// ui.some_component->someSetter(conf.some_variable);
}

