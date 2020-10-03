#include <QtCore/QString>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include "dbushelper.h"


DBUSHelper::DBUSHelper(){
}

DBUSHelper::~DBUSHelper(){
}

int DBUSHelper::notificationSimple(QString notification){
	QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "SystemNoteInfoprint");
	QList<QVariant> args; args.append(QVariant::fromValue(notification));
	msg.setArguments(args);
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		return -1;
	}
	return 0;
}

int DBUSHelper::notificationBig(QString notification){
	QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "SystemNoteDialog");
	QList<QVariant> args; args.append(QVariant::fromValue(notification)); args.append((unsigned int)0); args.append("OK");
	msg.setArguments(args);
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		return -1;
	}
	return 0;
}

int DBUSHelper::setCellRadioEnabled(bool enabled){
	/* -1=fail, 0=ok */
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.phone.SSC", "/com/nokia/phone/SSC", "com.nokia.phone.SSC", "set_radio");
	QList<QVariant> args; args.append(QVariant::fromValue(enabled));
	msg.setArguments(args);
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		return -1;
	}
	return 0;
}

int DBUSHelper::internetDisconnect(){
	/* -1=fail, 0=ok */
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "disconnect_req");
	QList<QVariant> args; args.append(QVariant::fromValue((unsigned int)32768));
	msg.setArguments(args);
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		return -1;
	}
	return 0;
}

int DBUSHelper::internetConnect(QString IAP_ID){
	/* -1=fail, 0=ok */
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.icd", "/com/nokia/icd", "com.nokia.icd", "connect");
	QList<QVariant> args; args.append(IAP_ID); args.append((unsigned int)0);
	msg.setArguments(args);
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		return -1;
	}
	return 0;
}

int DBUSHelper::cellInternetAvailable(bool *available){
	/* -1=fail, 0=ok */
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.phone.net", "/com/nokia/phone/net", "Phone.Net", "get_registration_status");
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	bool ok;
	char ret = -1;
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		ok = false;
	}
	else
		ret = msg2.arguments().value(0).toChar().toAscii();
	if(ok)
		*available = (ret == 0 || ret == 1 || ret == 2); // home || roaming || roaming_blink
	return ok ? 0 : -1;
}

int DBUSHelper::internetEnforcement2G3G(DBUS_Enforcement2G3G *enforcement){
	/* -1=fail, 0=ok */
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.phone.net", "/com/nokia/phone/net", "Phone.Net", "get_selected_radio_access_technology");
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	bool ok;
	int ret = -1;
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		ok = false;
	}
	else
		ret = msg2.arguments().value(0).toInt(&ok);
	if(ok){
		if(ret == 0)
			*enforcement = DBUS_2G3G_DOUBLE;
		else if(ret == 1)
			*enforcement = DBUS_2G3G_2G;
		else if(ret == 2)
			*enforcement = DBUS_2G3G_3G;
		else
			ok = false;
	}
	return ok ? 0 : -1;
}

int DBUSHelper::setInternetEnforcement2G3G(DBUS_Enforcement2G3G enforcement){
	/* -1=fail, 0=ok */
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.phone.net", "/com/nokia/phone/net", "Phone.Net", "set_selected_radio_access_technology");
	QList<QVariant> args;
	int arg;
	switch(enforcement){
	case DBUS_2G3G_2G:
		arg = 1;
		break;
	case DBUS_2G3G_3G:
		arg = 2;
		break;
	case DBUS_2G3G_DOUBLE:
		arg = 0;
		break;
	default:
		arg = 100;
	}
	args.append(QVariant::fromValue((unsigned char)arg));
	msg.setArguments(args);
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		return -1;
	}
	return 0;
}

int DBUSHelper::internetConnectionsCount(){
	/* -1=fail, ?=number of active connections */
	/* info : sends a state_req to dbus, a state_sig may be received by listeners */
	QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.icd2", "/com/nokia/icd2", "com.nokia.icd2", "state_req");
	QDBusMessage msg2 = QDBusConnection::systemBus().call(msg);
	bool ok;
	int ret = -1;
	if(msg2.type() == QDBusMessage::ErrorMessage){
		qDebug(msg2.errorMessage().toAscii().data());
		ok = false;
	}
	else
		ret = msg2.arguments().value(0).toInt(&ok);
	return ok ? ret : -1;
}




