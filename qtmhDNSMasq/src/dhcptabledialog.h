#ifndef DHCPTABLEDIALOG_H
#define DHCPTABLEDIALOG_H

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>
#include <QtCore/QStringList>
#include "pluginconfiguration.h"
#include "ui_dhcptabledialog.h"

class DHCPTableDialog : public QMainWindow
{
    Q_OBJECT

public:
	DHCPTableDialog(QWidget *parent = 0);
    ~DHCPTableDialog();

    void configuration(QStringList *conf);
    void setConfiguration(QStringList &conf);

protected:
	void closeEvent(QCloseEvent* event);

private:
    Ui::DHCPTableDialogClass ui;

    void tableToList(QStringList *list);
    void listToTable(QStringList list);

private slots:
	void loadDHCPTable();
	void saveDHCPTable();
	void addDHCP();
	void removeDHCP();

signals:
	void windowClosed();
};

#endif // DHCPTABLEDIALOG_H
