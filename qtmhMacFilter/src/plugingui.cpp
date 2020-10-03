#include <QtGui/QCloseEvent>
#include <QtGui/QMenuBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QFileDialog>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtGui/QMessageBox>
#include <QtCore/QList>
#include <QtCore/QListIterator>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QStringListIterator>
#include <QtCore/QChar>
#include "macinputdialog.h"
#include "plugingui.h"
#include "pluginconfiguration.h"


PluginGUI::PluginGUI(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);

	ui.lstMode->clear();
	ui.lstMode->addItem(tr("Allow those MACs"), "allow");
	ui.lstMode->addItem(tr("Deny those MACs"), "deny");
	lastClickedMac = 0;
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
	QString mode =  ui.lstMode->itemData(ui.lstMode->currentIndex()).toString();
	if(mode == "allow")
		conf->filterMode = MODE_ALLOW;
	else
		conf->filterMode = MODE_DENY;

	QStringList list;
	QList<QCheckBox*> boxes = ui.scroll->findChildren<QCheckBox*>();
	QListIterator<QCheckBox*> iterator(boxes);
	while(iterator.hasNext()){
		QCheckBox *box = iterator.next();
		QString entry = (box->isChecked() ? QString("yes") : QString("no")) + "/";
		QStringList params = box->text().split("/");
		entry += params.value(0).trimmed() + "/" + params.value(1).trimmed();
		list.append(entry);
	}
	conf->mactable = list;
}

void PluginGUI::setConfiguration(PluginConfiguration &conf){
	// Display configuration from the configuration object
	// ui.some_component->someSetter(conf.some_variable);
	if(conf.filterMode == MODE_ALLOW)
		ui.lstMode->setCurrentIndex(0);
	else
		ui.lstMode->setCurrentIndex(1);

	QStringListIterator iterator(conf.mactable);
	while(iterator.hasNext()){
		QString extracted = iterator.next();
		QStringList list = extracted.split("/");
		bool checked = list.value(0) == "yes";
		QString name = list.value(1);
		QString mac = list.value(2);
		QCheckBox *chk = new QCheckBox(this);
		chk->setChecked(checked);
		chk->setText(name + " / " + mac);
		QObject::connect(chk, SIGNAL(clicked()), this, SLOT(macClicked()));
		ui.vertical->addWidget(chk);
	}
}

void PluginGUI::macClicked(){
	QObject *sender = const_cast<QObject*>(QObject::sender());
	lastClickedMac = static_cast<QCheckBox*>(sender);
}

void PluginGUI::addMac(){
	MacInputDialog dlg;
	if(dlg.exec() != dlg.Accepted)
		return;
	QString name = dlg.machineName().trimmed().replace("/","").toLower();
	QString mac = dlg.machineMAC().trimmed().replace("/","").toLower();
	if(! isValidMAC(name, mac))
		return;
	QCheckBox *box = new QCheckBox(this);
	box->setChecked(true);
	box->setText(name + " / " + mac);
	QObject::connect(box, SIGNAL(clicked()), this, SLOT(macClicked()));
	ui.vertical->addWidget(box);
	lastClickedMac = 0;
}

bool PluginGUI::isValidMAC(QString name, QString mac){
	try{
		if(name == "")
			throw tr("Empty name");
		QStringList parts = mac.split(":");
		if(parts.size() != 6)
			throw tr("A MAC address comes in 6 parts separated by colons (:)");
		for(int i = 0; i < 6; i++){
			QString part = parts.at(i);
			if(part.size() != 2)
				throw tr("A part of a MAC has a length of 2");
			for(int j = 0; j < part.size(); j++){
				QChar digit = part.at(j);
				if( (digit < '0' || digit > '9') && (digit < 'a' || digit > 'f') )
					throw tr("Digits in a MAC go from 0 to f");
			}
		}
		return true;
	}
	catch(QString fail){
		QMessageBox::warning(this, tr("Invalid MAC"), ((tr("The given MAC/machine is invalid.") + "\n") + tr("Reason :") + " ") + fail);
		return false;
	}
}


void PluginGUI::removeMac(){
	if(lastClickedMac == 0)
		return;
	delete lastClickedMac;
	lastClickedMac = 0;
}

void PluginGUI::enableAll(){
	QList<QCheckBox*> boxes = ui.scroll->findChildren<QCheckBox*>();
	QListIterator<QCheckBox*> iterator(boxes);
	while(iterator.hasNext()){
		QCheckBox *box = iterator.next();
		box->setChecked(true);
	}
	lastClickedMac = 0;
}

void PluginGUI::disableAll(){
	QList<QCheckBox*> boxes = ui.scroll->findChildren<QCheckBox*>();
	QListIterator<QCheckBox*> iterator(boxes);
	while(iterator.hasNext()){
		QCheckBox *box = iterator.next();
		box->setChecked(false);
	}
	lastClickedMac = 0;
}

void PluginGUI::removeAllMACs(){
	QList<QCheckBox*> boxes = ui.scroll->findChildren<QCheckBox*>();
	QListIterator<QCheckBox*> iterator(boxes);
	while(iterator.hasNext()){
		QCheckBox *box = iterator.next();
		delete box;
	}
	lastClickedMac = 0;
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
	QString content = QString(file.readAll());
	file.close();
	removeAllMACs();
	try{
		QStringList list = content.split("/");
		QStringListIterator iterator(list);
		QString action = iterator.next();
		if(action == "allow")
			ui.lstMode->setCurrentIndex(0);
		else
			ui.lstMode->setCurrentIndex(1);
		while(iterator.hasNext()){
			QString active = iterator.next();
			if(active != "end"){
				QCheckBox *box = new QCheckBox(this);
				QObject::connect(box, SIGNAL(clicked()), this, SLOT(macClicked()));
				box->setChecked(active == "yes");
				if(! iterator.hasNext()){
					delete box;
					throw QString(tr("Invalid file content"));
				}
				QString name = iterator.next();
				if(! iterator.hasNext()){
					delete box;
					throw QString(tr("Invalid file content"));
				}
				QString mac = iterator.next();
				box->setText(name + " / " + mac);
				ui.vertical->addWidget(box);
			}
		}
	}
	catch(QString fail){
		QMessageBox::critical(this, tr("Error"), fail);
		removeAllMACs();
	}
	lastClickedMac = 0;
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
	QString content = ui.lstMode->itemData(ui.lstMode->currentIndex()).toString() + "/";
	QList<QCheckBox*> boxes = ui.scroll->findChildren<QCheckBox*>();
	QListIterator<QCheckBox*> iterator(boxes);
	while(iterator.hasNext()){
		QCheckBox *box = iterator.next();
		content += box->isChecked() ? "yes/" : "no/";
		QStringList list = box->text().split("/");
		content += list.at(0).trimmed() + "/";
		content += list.at(1).trimmed() + "/";
	}
	content += "end";
	file.write(content.toAscii());
	file.close();
}

