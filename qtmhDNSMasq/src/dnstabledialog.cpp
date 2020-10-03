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
#include "dnsdialog.h"
#include "dnstabledialog.h"

DNSTableDialog::DNSTableDialog(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);
}

DNSTableDialog::~DNSTableDialog()
{

}

void DNSTableDialog::closeEvent(QCloseEvent* event){
	// Signal that the GUI is closed
	if(menuBar() != 0)
		menuBar()->clear(); // Solves a bug where menubar in child gui is kept in parent gui
	emit windowClosed();
	event->accept();
}

void DNSTableDialog::configuration(QStringList *conf){
	tableToList(conf);
}

void DNSTableDialog::setConfiguration(QStringList &conf){
	listToTable(conf);
}

void DNSTableDialog::listToTable(QStringList list){
	ui.lstDNS->clear();
	QStringListIterator iterator(list);
	while(iterator.hasNext()){
		QString tuple = iterator.next();
		QStringList parts = tuple.split("/");
		if(parts.size() == 2){
			QString server = parts.at(0);
			QString ip = parts.at(1);
			QString text = server + "  ->  " + ip;
			QListWidgetItem *item = new QListWidgetItem(text, ui.lstDNS);
			item->setData(Qt::UserRole, tuple);
			ui.lstDNS->addItem(item);
		}
	}
}

void DNSTableDialog::tableToList(QStringList *list){
	list->clear();
	for(int row = 0; row < ui.lstDNS->count(); row++){
		QString entry = ui.lstDNS->item(row)->data(Qt::UserRole).toString();
		list->append(entry);
	}
}

void DNSTableDialog::loadDNSTable(){
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

void DNSTableDialog::saveDNSTable(){
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

void DNSTableDialog::addDNS(){
	DNSDialog dlg;
	if(dlg.exec() != dlg.Accepted)
		return;
	QString server = dlg.server;
	QString ip = dlg.ip;
	QString tuple = server + "/" + ip;
	QString text = server + "  ->  " + ip;
	QListWidgetItem *item = new QListWidgetItem(text, ui.lstDNS);
	item->setData(Qt::UserRole, tuple);
	ui.lstDNS->addItem(item);
}

void DNSTableDialog::removeDNS(){
	int row = ui.lstDNS->currentRow();
	if(row < 0)
		return;
	delete ui.lstDNS->item(row);
}
