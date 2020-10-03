#include <QtGui/QCloseEvent>
#include <QtGui/QMenuBar>
#include <QtCore/QString>
#include "pluginconfiguration.h"
#include "plugingui.h"


PluginGUI::PluginGUI(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);

	ui.lstUA->clear();
	ui.lstUA->addItem(tr("No change"), "nochange");
	ui.lstUA->addItem(tr("MicroB (N900 browser)"), "microb");
	ui.lstUA->addItem(tr("Iphone"), "iphone");
	ui.lstUA->addItem(tr("Custom User-Agent"), "custom");
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
	bool ok;
	conf->port = ui.txtPort->text().toInt(&ok);
	if(! ok)
		conf->port = 8118;
	conf->ports = ui.txtPorts->text();
	if(ui.lstUA->currentIndex() == 0)
		conf->ua = UA_NOCHANGE;
	else if(ui.lstUA->currentIndex() == 1)
		conf->ua = UA_MICROB;
	else if(ui.lstUA->currentIndex() == 2)
		conf->ua = UA_IPHONE;
	else
		conf->ua = UA_CUSTOM;
	conf->customUA = ui.txtCustomUA->text();
	conf->defaultActions = ui.chkDefaultActions->isChecked();
	conf->forward = ui.chkForward->isChecked();
	conf->forwardIP = ui.txtForwardIP->text().trimmed();
	if(conf->forwardIP == "")
		conf->forwardIP = "127.0.0.1";
	conf->forwardPort = ui.txtForwardPort->text().toInt(&ok);
	if(! ok)
		conf->forwardPort = 10000;
}

void PluginGUI::setConfiguration(PluginConfiguration &conf){
	// Display configuration from the configuration object
	// ui.some_component->someSetter(conf.some_variable);

	ui.txtPort->setText(QString::number(conf.port));
	ui.txtPorts->setText(conf.ports);
	if(conf.ua == UA_NOCHANGE)
		ui.lstUA->setCurrentIndex(0);
	else if(conf.ua == UA_MICROB)
		ui.lstUA->setCurrentIndex(1);
	else if(conf.ua == UA_IPHONE)
		ui.lstUA->setCurrentIndex(2);
	else
		ui.lstUA->setCurrentIndex(3);
	ui.txtCustomUA->setText(conf.customUA);
	ui.chkDefaultActions->setChecked(conf.defaultActions);
	ui.chkForward->setChecked(conf.forward);
	ui.txtForwardIP->setText(conf.forwardIP);
	ui.txtForwardPort->setText(QString::number(conf.forwardPort));
}



