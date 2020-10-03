#ifndef MACINPUTDIALOG_H
#define MACINPUTDIALOG_H

#include <QtGui/QDialog>
#include "ui_macinputdialog.h"

class MacInputDialog : public QDialog
{
    Q_OBJECT

public:
    MacInputDialog(QWidget *parent = 0);
    ~MacInputDialog();
    QString machineName();
    QString machineMAC();

private:
    Ui::MacInputDialogClass ui;
};

#endif // MACINPUTDIALOG_H
