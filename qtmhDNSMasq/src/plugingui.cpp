#include <QtGui/QCloseEvent>
#include <QtGui/QMenuBar>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "dnstabledialog.h"
#include "dhcptabledialog.h"
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
	conf->domainName = ui.txtDomainName->text().trimmed();
	conf->noHosts = ui.chkNoHosts->isChecked();
	conf->dnsTable = dnsTable;
	conf->dhcpTable = dhcpTable;
	conf->dhcpFrom = ui.txtDHCPFrom->text();
	conf->dhcpTo = ui.txtDHCPTo->text();
}

void PluginGUI::setConfiguration(PluginConfiguration &conf){
	// Display configuration from the configuration object
	// ui.some_component->someSetter(conf.some_variable);
	ui.txtDomainName->setText(conf.domainName);
	ui.chkNoHosts->setChecked(conf.noHosts);
	dnsTable = conf.dnsTable;
	dhcpTable = conf.dhcpTable;
	ui.txtDHCPFrom->setText(conf.dhcpFrom);
	ui.txtDHCPTo->setText(conf.dhcpTo);
}

void PluginGUI::dnsTableShow(){
	dnstableDialog = new DNSTableDialog(this);
	dnstableDialog->setConfiguration(dnsTable);
	connect(dnstableDialog, SIGNAL(windowClosed()), this, SLOT(dnsTableClosed()));
	dnstableDialog->show();
}

void PluginGUI::dnsTableClosed(){
	dnstableDialog->configuration(&dnsTable);
}

void PluginGUI::dhcpTableShow(){
	dhcptableDialog = new DHCPTableDialog(this);
	dhcptableDialog->setConfiguration(dhcpTable);
	connect(dhcptableDialog, SIGNAL(windowClosed()), this, SLOT(dhcpTableClosed()));
	dhcptableDialog->show();
}

void PluginGUI::dhcpTableClosed(){
	dhcptableDialog->configuration(&dhcpTable);
}

