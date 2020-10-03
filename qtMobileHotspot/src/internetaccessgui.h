#ifndef INTERNETACCESSGUI_H
#define INTERNETACCESSGUI_H

#define TIME_APNS ((int)2500)

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtDBus/QDBusMessage>
#include "mobilehotspotconfiguration.h"
#include "ui_internetaccessgui.h"


class InternetAccessGUI : public QMainWindow
{
    Q_OBJECT

public:
    InternetAccessGUI(QWidget *parent = 0);
    ~InternetAccessGUI();
    void setConfiguration(MobileHotspotConfiguration &configuration);
    void configuration(MobileHotspotConfiguration *configuration);

protected:
	void closeEvent(QCloseEvent* event);

private:
	void apnsEnableSignal();
	void apnsDisableSignal();
	bool apnsRequest();
	bool apnsCancelRequest();
	QString validNetwork();

    Ui::InternetAccessGUIClass ui;
    bool gprsOnly;
    QString currentIAPID;
    bool waitingForAPNs;
    QList<QStringList> apns;

private slots :
	void apnsSignal(const QDBusMessage &message);
	void apnsTimeout();
	void updateAPNs();

signals:
	void windowClosed();
};

#endif // INTERNETACCESSGUI_H
