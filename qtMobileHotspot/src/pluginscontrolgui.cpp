#include <QtGui/QCloseEvent>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QListIterator>
#include <QtGui/QFont>
#include <QtGui/QIcon>
#include <QtGui/QListWidgetItem>
#include "mobilehotspotconfiguration.h"
#include "mobilehotspotplugin.h"
#include "pluginscontrolgui.h"


PluginsControlGUI::PluginsControlGUI(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);
}

PluginsControlGUI::~PluginsControlGUI()
{

}

void PluginsControlGUI::closeEvent(QCloseEvent* event){
	// Signal that the GUI is closed
	if(menuBar() != 0)
		menuBar()->clear(); // Solves a bug where menubar in child gui is kept in parent gui
	emit windowClosed();
	event->accept();
}

void PluginsControlGUI::setConfiguration(MobileHotspotConfiguration *configuration, bool running, QMap<QString, MobileHotspotPlugin*> *plugins, QList<QString> *sortedPlugins){
	this->configuration = configuration;
	this->running = running;
	this->plugins = plugins;
	this->sortedPlugins = sortedPlugins;

	ui.lstPlugins->clear();
	QListIterator<QString> iterator(*sortedPlugins);
	while(iterator.hasNext()){
		QString id = iterator.next();
		MobileHotspotPlugin *plugin = plugins->value(id);
		qApp->installTranslator(plugin->translator());
			QString name = plugin->displayedName();
			QIcon icon = plugin->displayedIcon();
			bool enabled = plugin->isPluginEnabled();
		qApp->removeTranslator(plugin->translator());
		if(icon.isNull())
			icon = QIcon("./util/plugin-default-icon.png");

		QListWidgetItem *item = new QListWidgetItem(name);
		item->setIcon(icon);
		if(enabled)
			item->setForeground(QApplication::palette().highlight());
		else
			item->setForeground(QApplication::palette().mid());
		ui.lstPlugins->addItem(item);
	}
	if(ui.lstPlugins->count() > 0)
		ui.lstPlugins->setCurrentRow(0);
}

void PluginsControlGUI::pluginSelected(QListWidgetItem *current, QListWidgetItem *previous){
	QFont font;
	if(previous != 0){
		font = previous->font();
		font.setBold(false);
		previous->setFont(font);
	}
	if(current != 0){
		font = current->font();
		font.setBold(true);
		current->setFont(font);
	}

	int row = ui.lstPlugins->row(current);
	ui.lstPlugins->setCurrentRow(row);
	QString id = sortedPlugins->at(row);
	MobileHotspotPlugin *plugin = plugins->value(id);
	qApp->installTranslator(plugin->translator());
		QString name = plugin->displayedName();
		QString author = plugin->displayedAuthor();
		QString contact = plugin->displayedContact();
		QString descrition = plugin->displayedDescription();
		bool enabled = plugin->isPluginEnabled();
	qApp->removeTranslator(plugin->translator());

	ui.lblName->setText(name);
	ui.lblAuthor->setText(author);
	ui.lblContact->setText(contact);
	ui.lblDescription->setText(descrition);
	ui.chkEnabled->setChecked(enabled);
	ui.chkEnabled->setEnabled((plugin->allowEnableDisableRunning() && running) || (plugin->allowEnableDisableStopped() && ! running));
	ui.btnConfigure->setEnabled((plugin->allowConfigureRunning() && running) || (plugin->allowConfigureStopped() && ! running));
	ui.btnReset->setEnabled(ui.btnConfigure->isEnabled());
}

void PluginsControlGUI::configurePlugin(){
	QString id = sortedPlugins->at(ui.lstPlugins->currentRow());
	MobileHotspotPlugin *plugin = plugins->value(id);
	qDebug("Plugin configure : %s", plugin->name().toAscii().data());
	plugin->configure(this, configuration, running, false); // translators are handled by plugins themselves here
}

void PluginsControlGUI::resetPlugin(){
	if(QMessageBox::question(this, tr("Reset plugin"), tr("Do you really want to reset this plugin ?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;
	QString id = sortedPlugins->at(ui.lstPlugins->currentRow());
	MobileHotspotPlugin *plugin = plugins->value(id);
	qDebug("Plugin %s : reset", plugin->name().toAscii().data());
	plugin->configure(this, configuration, running, true); // translators are handled by plugins themselves here
}

void PluginsControlGUI::enableDisablePlugin(){
	bool enabled = ui.chkEnabled->isChecked();
	QString id = sortedPlugins->at(ui.lstPlugins->currentRow());
	MobileHotspotPlugin *plugin = plugins->value(id);
	qDebug("Plugin %s : %s", enabled ? "enable" : "disable", plugin->name().toAscii().data());
	qApp->installTranslator(plugin->translator());
		plugin->setPluginEnabled(enabled, configuration, running);
		enabled = plugin->isPluginEnabled();
	qApp->removeTranslator(plugin->translator());

	ui.chkEnabled->setChecked(enabled);
	QListWidgetItem *item = ui.lstPlugins->item(ui.lstPlugins->currentRow());
	item->setForeground(enabled ? QApplication::palette().highlight() : QApplication::palette().mid());
}
