#include <QtCore/QString>
#include <QtGui/QMessageBox>
#include "dnsdialog.h"

DNSDialog::DNSDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

DNSDialog::~DNSDialog()
{

}

void DNSDialog::beforeAccept(){
	server = ui.txtServer->text().trimmed();
	ip = ui.txtIP->text().trimmed();
	if(server == "" || ip == ""){
		QMessageBox::warning(this, tr("Invalid entry"), tr("Your entry is invalid, please correct it."));
		return;
	}
	this->accept();
}
