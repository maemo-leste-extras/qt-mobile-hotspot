#ifndef DBUSHELPER_H
#define DBUSHELPER_H

#include <QtCore/QString>
#include <QtDBus/QDBusMessage>


enum DBUS_Enforcement2G3G{
	DBUS_2G3G_2G,
	DBUS_2G3G_3G,
	DBUS_2G3G_DOUBLE
};


class DBUSHelper
{

public:
	DBUSHelper();
    ~DBUSHelper();
    static int notificationSimple(QString notification);
    static int notificationBig(QString notification);
    static int setCellRadioEnabled(bool enabled);
    static int internetDisconnect();
    static int internetConnect(QString IAP_ID);
    static int cellInternetAvailable(bool *available);
    static int internetEnforcement2G3G(DBUS_Enforcement2G3G *enforcement);
    static int setInternetEnforcement2G3G(DBUS_Enforcement2G3G enforcement);
    static int internetConnectionsCount();
};

#endif // DBUSHELPER_H
