#include <QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtCore/QString>
#include "mobilehotspotconfiguration.h"
#include "accesspointgui.h"


AccessPointGUI::AccessPointGUI(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Maemo5StackedWindow);

	ui.lstMAC->addItem(tr("No change to MAC"));
	ui.lstMAC->addItem(tr("Use custom MAC :"));
	ui.lstMAC->addItem(tr("Use a random MAC"));
}

AccessPointGUI::~AccessPointGUI()
{

}

void AccessPointGUI::closeEvent(QCloseEvent* event){
	// Signal that the GUI is closed
	if(menuBar() != 0)
		menuBar()->clear(); // Solves a bug where menubar in child gui is kept in parent gui
	emit windowClosed();
	event->accept();
}

void AccessPointGUI::setConfiguration(MobileHotspotConfiguration &configuration){
	ui.txtAPName->setText(configuration.hotspotName);
	ui.chkEncryption->setChecked(configuration.encryptionType == configuration.ENCRYPTION_WEP);
	ui.txtKey->setText(configuration.encryptionKey);
	if(configuration.wlanTypeMAC == MAC_NORMAL)
		ui.lstMAC->setCurrentIndex(0);
	else if(configuration.wlanTypeMAC == MAC_CUSTOM)
		ui.lstMAC->setCurrentIndex(1);
	else
		ui.lstMAC->setCurrentIndex(2);
	ui.txtMAC->setText(configuration.wlanCustomMAC);
	ui.chkWLANDriver->setChecked(configuration.cycleWlanDriver);
}

void AccessPointGUI::configuration(MobileHotspotConfiguration *configuration){
	QString name = ui.txtAPName->text().replace("\\", "").replace("\"", "");
	configuration->hotspotName = name == "" ? "N900 Hotspot" : name;
	configuration->encryptionType = ui.chkEncryption->isChecked() ? configuration->ENCRYPTION_WEP : configuration->ENCRYPTION_NONE;
	configuration->encryptionKey = validKey();
	if(ui.lstMAC->currentIndex() == 0)
		configuration->wlanTypeMAC = MAC_NORMAL;
	else if(ui.lstMAC->currentIndex() == 1)
		configuration->wlanTypeMAC = MAC_CUSTOM;
	else
		configuration->wlanTypeMAC = MAC_RANDOM;
	configuration->wlanCustomMAC = validMAC();
	configuration->cycleWlanDriver = ui.chkWLANDriver->isChecked();
}

QString AccessPointGUI::validKey(){
	QString res;
	QString entered = ui.txtKey->text();
	try{
		if(entered.size() != 5 && entered.size() != 13)
			throw QString("fail");
		res = entered;
	}
	catch(QString fail){
		QMessageBox::warning(this, tr("Invalid key"), tr("Keys must have a length of 5 or 13. Your key has been tweaked according to this."));
		if(entered.size() < 5)
			res = entered + QString(5 - entered.size(), '0');
		else if(entered.size() < 13)
			res = entered + QString(13 - entered.size(), '0');
		else
			res = entered.left(13);
	}
	return res;
}

QString AccessPointGUI::validMAC(){
	QString mac = ui.txtMAC->text();
	try{
		QStringList parts = mac.split(":");
		if(parts.size() != 6)
			throw QString();
		for(int i = 0; i < 6; i++){
			QString part = parts.at(i);
			if(part.size() != 2)
				throw QString();
			for(int j = 0; j < part.size(); j++){
				QChar digit = part.at(j);
				if( (digit < '0' || digit > '9') && (digit < 'a' || digit > 'f') )
					throw QString();
			}
		}
		return mac;
	}
	catch(QString fail){
		QMessageBox::warning(this, tr("Invalid MAC"), tr("The custom MAC is invalid. It has been reset to the default value."));
		return "02:11:22:33:44:55";
	}
}
