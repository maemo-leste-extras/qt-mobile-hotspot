#include <QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QListIterator>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtCore/QTimer>
#include "mobilehotspotconfiguration.h"
#include "apnhelper.h"
#include "internetaccessgui.h"


InternetAccessGUI::InternetAccessGUI(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);
	ui.lst2G3G->addItem(tr("No change"), "ignore");
	ui.lst2G3G->addItem(tr("2G only"), "2g");
	ui.lst2G3G->addItem(tr("3G only"), "3g");
	ui.lst2G3G->addItem(tr("2G/3G"), "2g3g");

	waitingForAPNs = false;
	gprsOnly = false;
	currentIAPID = "";
}

InternetAccessGUI::~InternetAccessGUI()
{

}

void InternetAccessGUI::closeEvent(QCloseEvent* event){
	// Signal that the GUI is closed
	if(menuBar() != 0)
		menuBar()->clear(); // Solves a bug where menubar in child gui is kept in parent gui

	if(waitingForAPNs){
		waitingForAPNs = false;
		apnsCancelRequest();
		apnsDisableSignal();
	}

	emit windowClosed();
	event->accept();
}

void InternetAccessGUI::setConfiguration(MobileHotspotConfiguration &configuration){
	ui.txtNetwork->setText(configuration.lanNetwork + "0");
	ui.chkInternet->setChecked(configuration.internetEnabled);
	if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_IGNORE)
		ui.lst2G3G->setCurrentIndex(0);
	else if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_2G)
		ui.lst2G3G->setCurrentIndex(1);
	else if(configuration.enforcement2g3g == ENFORCEMENT_2G3G_3G)
		ui.lst2G3G->setCurrentIndex(2);
	else
		ui.lst2G3G->setCurrentIndex(3);

	gprsOnly = configuration.lanInterface == configuration.INTERFACE_WLAN;
	currentIAPID = configuration.internetAPID;
	updateAPNs();
}

void InternetAccessGUI::updateAPNs(){
	ui.lstAPN->clear();
	apns.clear();
	apnsEnableSignal();
	if(apnsRequest()){
		waitingForAPNs = true;
		ui.lstAPN->setEnabled(false);
		ui.btnRefresh->setEnabled(false);
		QTimer::singleShot(TIME_APNS, this, SLOT(apnsTimeout()));
	}
	else{
		qDebug("WARNING : Using saved APNs instead of available ones");
		int configIndex = 0;
		apns = APNHelper::apnList(gprsOnly);
		for(int i = 0; i < apns.size(); i++){
			ui.lstAPN->addItem( (apns.at(i).at(0) + "     ") + apns.at(i).at(2), apns.at(i).at(1));
			if(currentIAPID == apns.at(i).at(1))
				configIndex = i;
		}
		ui.lstAPN->setCurrentIndex(configIndex);
	}
}

void InternetAccessGUI::configuration(MobileHotspotConfiguration *configuration){
	if(ui.lstAPN->count() > 0){
		configuration->internetAPName = ui.lstAPN->itemText(ui.lstAPN->currentIndex()).split("     ").at(0);
		configuration->internetAPType = ui.lstAPN->itemText(ui.lstAPN->currentIndex()).split("     ").at(1);
		configuration->internetAPID = ui.lstAPN->itemData(ui.lstAPN->currentIndex()).toString();
	}
	configuration->lanNetwork = validNetwork();
	configuration->internetEnabled = ui.chkInternet->isChecked();
	if(ui.lst2G3G->currentIndex() == 0)
		configuration->enforcement2g3g = ENFORCEMENT_2G3G_IGNORE;
	else if(ui.lst2G3G->currentIndex() == 1)
		configuration->enforcement2g3g = ENFORCEMENT_2G3G_2G;
	else if(ui.lst2G3G->currentIndex() == 2)
		configuration->enforcement2g3g = ENFORCEMENT_2G3G_3G;
	else
		configuration->enforcement2g3g = ENFORCEMENT_2G3G_DOUBLE;
}

void InternetAccessGUI::apnsDisableSignal(){
	QDBusConnection dbus = QDBusConnection::systemBus();
	dbus.disconnect("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "scan_result_sig", this, SLOT(apnsSignal(const QDBusMessage&)));
}

void InternetAccessGUI::apnsEnableSignal(){
	QDBusConnection dbus = QDBusConnection::systemBus();
	dbus.connect("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "scan_result_sig", this, SLOT(apnsSignal(const QDBusMessage&)));
}

bool InternetAccessGUI::apnsRequest(){
	/** true=ok, false=fail **/
	QDBusConnection dbus = QDBusConnection::systemBus();
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "scan_req");
	QList<QVariant> args;
	args.append(QVariant::fromValue((uint)0)); // ICD_SCAN_REQUEST_ACTIVE
	msg.setArguments(args);
	QDBusMessage msg2 = dbus.call(msg);
	if(msg2.type() == msg2.ErrorMessage){
		qDebug("WARNING : Was not able to get available APNs (%s)", msg2.errorMessage().toLatin1().data());
		return false;
	}
	else
		return true;
}

bool InternetAccessGUI::apnsCancelRequest(){
	/** true=ok, false=fail **/
	QDBusConnection dbus = QDBusConnection::systemBus();
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "scan_cancel_req");
	QDBusMessage msg2 = dbus.call(msg);
	if(msg2.type() == msg2.ErrorMessage){
		qDebug("WARNING : Was not able to cancel APN scan (%s)", msg2.errorMessage().toLatin1().data());
		return false;
	}
	else
		return true;
}

void InternetAccessGUI::apnsSignal(const QDBusMessage &message){
	if(! waitingForAPNs)
		return;
	QList<QVariant> args = message.arguments();
	if(message.arguments().size() != 15)
		return;
	QString networkID = args.at(10).toByteArray();
	QString networkType = args.at(7).toString();
	QString networkName = args.at(8).toString();
	if(networkID.trimmed() != "" && networkType.trimmed() != "" && networkName.trimmed() != "")
		if( (gprsOnly && networkType == "GPRS") || (! gprsOnly) ){
			QStringList apn;
			apn.append(networkName);
			apn.append(networkID);
			apn.append(networkType);
			apns.append(apn);
		}
}

void InternetAccessGUI::apnsTimeout(){
	if(! waitingForAPNs)
		return;
	apnsCancelRequest();
	apnsDisableSignal();
	waitingForAPNs = false;
	ui.lstAPN->setEnabled(true);
	ui.btnRefresh->setEnabled(true);
	int configIndex = 0;
	QListIterator<QStringList> iterator(apns);
	while(iterator.hasNext()){
		QStringList apn = iterator.next();
		QString text = (apn.at(0) + "     ") + apn.at(2);
		QString data = apn.at(1);
		if(ui.lstAPN->findData(data) == -1){
			if(currentIAPID == data)
				configIndex = ui.lstAPN->count();
			ui.lstAPN->addItem(text, data);
		}
	}
	ui.lstAPN->setCurrentIndex(configIndex);
}

QString InternetAccessGUI::validNetwork(){
	QStringList list = ui.txtNetwork->text().split('.');
	QString res;
	try{
		if(list.size() != 4)
			throw QString("fail");
		for(int i = 0; i < 3; i++){
			bool ok;
			int n = list.at(i).toInt(&ok);
			ok = ok && n >= 0 && n <= 255;
			if(! ok)
				throw QString("fail");
			res += QString::number(n) + ".";
		}
	}
	catch(QString fail){
		QMessageBox::warning(this, tr("Invalid network"), tr("The entered network is invalid. The default value will be used."));
		res = "10.40.160.";
	}
	return res;
}
