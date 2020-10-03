#include <QtGui/QCloseEvent>
#include <QtGui/QMenuBar>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QStringListIterator>
#include <QtGui/QFileDialog>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidgetItem>
#include "dhcpdialog.h"
#include "dhcptabledialog.h"

DHCPTableDialog::DHCPTableDialog(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);
}

DHCPTableDialog::~DHCPTableDialog()
{

}

void DHCPTableDialog::closeEvent(QCloseEvent* event){
	// Signal that the GUI is closed
	if(menuBar() != 0)
		menuBar()->clear(); // Solves a bug where menubar in child gui is kept in parent gui
	emit windowClosed();
	event->accept();
}

void DHCPTableDialog::configuration(QStringList *conf){
	tableToList(conf);
}

void DHCPTableDialog::setConfiguration(QStringList &conf){
	listToTable(conf);
}

void DHCPTableDialog::listToTable(QStringList list){
	ui.lstDHCP->clear();
	QStringListIterator iterator(list);
	while(iterator.hasNext()){
		QString tuple = iterator.next();
		QStringList parts = tuple.split("/");
		if(parts.size() == 3){
			QString hostname = parts.at(0);
			QString mac = parts.at(1);
			QString ip = parts.at(2);
			QString text;
			if(hostname == "")
				text = mac + " -> " + ip;
			else if(mac == "")
				text = hostname + " -> " + ip;
			else
				text = hostname + " (" + mac + ") ->  " + ip;
			QListWidgetItem *item = new QListWidgetItem(text, ui.lstDHCP);
			item->setData(Qt::UserRole, tuple);
			ui.lstDHCP->addItem(item);
		}
	}
}

void DHCPTableDialog::tableToList(QStringList *list){
	list->clear();
	for(int row = 0; row < ui.lstDHCP->count(); row++){
		QString entry = ui.lstDHCP->item(row)->data(Qt::UserRole).toString();
		list->append(entry);
	}
}

void DHCPTableDialog::loadDHCPTable(){
	QString path = QFileDialog::getOpenFileName(this, tr("Select a file..."), "/home/user");
	if(path == QString::null)
		return;
	QFile file(path);
	if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
		QMessageBox::critical(this, tr("Error"), tr("Cannot open the selected file"));
		return;
	}
	QString readMe(file.readAll());
	file.close();
	QStringList list = readMe.split("\n");
	listToTable(list);
}

void DHCPTableDialog::saveDHCPTable(){
	QString path = QFileDialog::getSaveFileName(this, tr("Select a file..."), "/home/user");
	if(path == QString::null)
		return;
	QFile file(path);
	if(! file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
		QMessageBox::critical(this, tr("Error"), tr("Cannot open the selected file"));
		return;
	}
	QString writeMe = "";
	QStringList list;
	tableToList(&list);
	QStringListIterator iterator(list);
	while(iterator.hasNext()){
		QString tuple = iterator.next();
		writeMe = (writeMe == "" ? tuple : ((writeMe + "\n") + tuple));
	}
	file.write(writeMe.toAscii());
	file.close();
}

void DHCPTableDialog::addDHCP(){
	DHCPDialog dlg;
	if(dlg.exec() != dlg.Accepted)
		return;
	QString hostname = dlg.hostname;
	QString mac = dlg.mac;
	QString ip = dlg.ip;
	QString tuple = hostname + "/" + mac + "/" + ip;
	QString text;
	if(hostname == "")
		text = mac + " -> " + ip;
	else if(mac == "")
		text = hostname + " -> " + ip;
	else
		text = hostname + " (" + mac + ") ->  " + ip;
	QListWidgetItem *item = new QListWidgetItem(text, ui.lstDHCP);
	item->setData(Qt::UserRole, tuple);
	ui.lstDHCP->addItem(item);
}

void DHCPTableDialog::removeDHCP(){
	int row = ui.lstDHCP->currentRow();
	if(row < 0)
		return;
	delete ui.lstDHCP->item(row);
}
