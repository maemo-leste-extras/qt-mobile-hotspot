#include <QtGui/QCloseEvent>
#include <QtGui/QMenuBar>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
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
	conf->list = ui.txtIptables->toPlainText().trimmed().split("\n");
}

void PluginGUI::setConfiguration(PluginConfiguration &conf){
	// Display configuration from the configuration object
	// ui.some_component->someSetter(conf.some_variable);
	ui.txtIptables->setPlainText(conf.list.join("\n"));
}

void PluginGUI::load(){
	QString path = QFileDialog::getOpenFileName(this, tr("Select a file..."), "/home/user");
	if(path == QString::null)
		return;
	QFile file(path);
	if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
		QMessageBox::critical(this, tr("Error"), tr("Cannot open the selected file"));
		return;
	}
	ui.txtIptables->setPlainText(QString(file.readAll()));
	file.close();
}

void PluginGUI::save(){
	QString path = QFileDialog::getSaveFileName(this, tr("Select a file..."), "/home/user");
	if(path == QString::null)
		return;
	QFile file(path);
	if(! file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
		QMessageBox::critical(this, tr("Error"), tr("Cannot open the selected file"));
		return;
	}
	file.write(ui.txtIptables->toPlainText().toAscii());
	file.close();
}

