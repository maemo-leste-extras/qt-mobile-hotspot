#ifndef DNSDIALOG_H
#define DNSDIALOG_H

#include <QtGui/QDialog>
#include "ui_dnsdialog.h"

class DNSDialog : public QDialog
{
    Q_OBJECT

public:
    DNSDialog(QWidget *parent = 0);
    ~DNSDialog();
    QString server;
    QString ip;

private:
    Ui::DNSDialogClass ui;

private slots:
    void beforeAccept();
};

#endif // DNSDIALOG_H
