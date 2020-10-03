#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>
#include <QtCore/QStringList>
#include "pluginconfiguration.h"
#include "dnstabledialog.h"
#include "dhcptabledialog.h"
#include "ui_plugingui.h"

class PluginGUI : public QMainWindow
{
    Q_OBJECT

public:
    PluginGUI(QWidget *parent = 0);
    ~PluginGUI();

    void configuration(PluginConfiguration *conf);
	void setConfiguration(PluginConfiguration &conf);

protected:
	void closeEvent(QCloseEvent* event);

private:
    Ui::PluginGUIClass ui;
    DNSTableDialog *dnstableDialog;
    DHCPTableDialog *dhcptableDialog;

    QStringList dnsTable;
    QStringList dhcpTable;

private slots:
	void dnsTableShow();
	void dnsTableClosed();
	void dhcpTableShow();
	void dhcpTableClosed();

signals:
	void windowClosed();
};

#endif // PLUGINGUI_H
