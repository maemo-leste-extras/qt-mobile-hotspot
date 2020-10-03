#ifndef DHCPDIALOG_H
#define DHCPDIALOG_H

#include <QtGui/QDialog>
#include "ui_dhcpdialog.h"

class DHCPDialog : public QDialog
{
    Q_OBJECT

public:
    DHCPDialog(QWidget *parent = 0);
    ~DHCPDialog();

    QString hostname;
    QString mac;
	QString ip;

private:
    Ui::DHCPDialogClass ui;

private slots:
    void beforeAccept();
};

#endif // DHCPDIALOG_H
