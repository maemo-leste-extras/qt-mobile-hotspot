#include <QtCore/QString>
#include <QtGui/QMessageBox>
#include "dhcpdialog.h"

DHCPDialog::DHCPDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

DHCPDialog::~DHCPDialog()
{

}

void DHCPDialog::beforeAccept(){
	hostname = ui.txtHostname->text();
	mac = ui.txtMAC->text();
	ip = ui.txtIP->text();
	// hostname or mac can be ommited (not both) according to DNSMasq manual
	if( (hostname == "" && mac == "") || (ip == "") ){
		QMessageBox::warning(this, tr("Invalid entry"), tr("Your entry is invalid, please correct it."));
		return;
	}
	this->accept();
}
