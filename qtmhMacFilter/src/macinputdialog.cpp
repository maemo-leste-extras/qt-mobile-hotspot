#include "macinputdialog.h"

MacInputDialog::MacInputDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

MacInputDialog::~MacInputDialog()
{

}

QString MacInputDialog::machineName(){
	return ui.txtName->text();
}

QString MacInputDialog::machineMAC(){
	return ui.txtMAC->text();
}

