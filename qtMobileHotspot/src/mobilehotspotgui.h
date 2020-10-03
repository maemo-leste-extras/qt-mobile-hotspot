#ifndef MOBILEHOTSPOTGUI_H
#define MOBILEHOTSPOTGUI_H

#include <QMainWindow>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtGui/QCloseEvent>
#include <QtCore/QProcess>
#include <QtCore/QMap>
#include <QAction>
#include <QtDBus/QDBusMessage>
#include <QtCore/QTranslator>
#include "dbushelper.h"
#include "mobilehotspotconfiguration.h"
#include "mobilehotspotplugin.h"
#include "pluginscontrolgui.h"
#include "accesspointgui.h"
#include "internetaccessgui.h"
#include "ui_mobilehotspotgui.h"


class MobileHotspotGUI : public QMainWindow
{
    Q_OBJECT

public:
    MobileHotspotGUI(QWidget *parent = 0);
    ~MobileHotspotGUI();

private:
	void updateDisplayedConfiguration();
	void loadPlugins();
	void sortPlugins();
	void updateServicesPlugins();
	void previousConnectionEnableSignal();
	void previousConnectionDisableSignal();
	void previousConnectionRequest();

	Ui::MobileHotspotGUIClass ui;
	QTranslator translator;

	PluginsControlGUI *pluginsControlGUI;
	AccessPointGUI *accessPointGUI;
	InternetAccessGUI *internetAccessGUI;

    QMap<QString, MobileHotspotPlugin*> plugins;
    QList<QString> sortedPlugins;
    QMap<QString, QString> pluginsTranslatorsBaseNames;
    QMap<QString, MobileHotspotPlugin*> servicesPlugins;

    MobileHotspotConfiguration configuration;

    DBUS_Enforcement2G3G previousEnforcement;
    QString previousWlanMAC;
    QStringList alreadyLoadedModules;
    int scanInterval;
    QProcess *dnsmasqProcess;
    int ipforward;

    bool showNotifications;
    bool successStarting;

    bool waitingForPreviousConnection;
    QString previousConnectionID;
    QString previousConnectionType;

protected :
        void closeEvent(QCloseEvent* event);

private slots:
	void usbWindowsAssistant();
	void configureLanguage();
	void configureLanguageFinished(bool saveConfiguration);
	void configurePlugins();
	void configurePluginsFinished();
	void configureAccessPoint();
	void configureAccessPointFinished();
	void configureInternetAccess();
	void configureInternetAccessFinished();
	void configureInterface(int index);
	void start(bool notification = true);
	void startProcedure();
	void stop(bool notification = true);
	void stopProcedure();
	void previousConnectionSignal(const QDBusMessage &message);
};

#endif // MOBILEHOTSPOTGUI_H
