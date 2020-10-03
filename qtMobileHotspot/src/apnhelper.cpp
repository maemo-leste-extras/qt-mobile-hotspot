#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QListIterator>
#include <gq/gconfitem.h>
#include "apnhelper.h"


APNHelper::APNHelper(){

}

APNHelper::~APNHelper(){

}

QList<QStringList> APNHelper::apnList(bool gprsOnly){
	QList<QStringList> list;
	GConfItem listroot(GCONF_APN_LIST);
	QList<QString> apns = listroot.listDirs();
	for(int i = 0; i < apns.size(); i++){
		QString apnkey = apns.at(i);
		QString apid = apnkey.right(apnkey.size() - apnkey.lastIndexOf('/') - 1).replace("@32@", " ");
		GConfItem apnType(apnkey + GCONF_APN_TYPE);
		QString type = apnType.value("?").toString();
		GConfItem apnName(apnkey + GCONF_APN_NAME);
		QString name = apnName.value("?").toString();
		if(name != "?" && type != "?" && !(gprsOnly && type != "GPRS")){
			QStringList sublist;
			sublist.append(name);
			sublist.append(apid);
			sublist.append(type);
			list.append(sublist);
			qDebug( (((QString("APN found : ") + name + " :: " + apid) + " :: ") + type).toAscii().data() );
		}
	}
	return sortAPNList(list);
}

QList<QStringList> APNHelper::sortAPNList(QList<QStringList> apns){
	QList<QStringList> list;
	QListIterator<QStringList> iterator(apns);
	while(iterator.hasNext()){
		QStringList apn = iterator.next();
		if(apn.at(2) == GCONF_APN_TYPE_GPRS)
			list.prepend(apn);
		else
			list.append(apn);
	}
	return list;
}

void APNHelper::setScanInterval(int newValue){
	GConfItem item(GCONF_SCANINTERVAL);
	item.set(newValue);
	qDebug( (QString("Set scan interval to ") + QString::number(newValue) ).toAscii().data() );
}

int APNHelper::scanInterval(){
	GConfItem item(GCONF_SCANINTERVAL);
	bool ok;
	int ret;
	ret = item.value(300).toInt(&ok); // 300 = 5 minutes, shouldnt be used anyway
	if(! ok)
		ret = 300;
	qDebug( (QString("Scan interval is ") + QString::number(ret)).toAscii().data() );
	return ret;
}

void APNHelper::setAPNPowerSave(QString apid, int level){
	QString realid = apid.replace(" ", "@32@");
	if(level == GCONF_APN_POWER_INVALID){
		qDebug("Tried to set an invalid power management for %s, ignoring", realid.toAscii().data());
		return;
	}
	GConfItem item( (QString(GCONF_APN_LIST) + "/") + realid + GCONF_APN_POWER);
	if(level == GCONF_APN_POWER_NOTFOUND){
		item.unset();
		qDebug("Unset the power management of %s", realid.toAscii().data());
	}
	else{
		item.set(level);
		qDebug("Set the power management of %s to %d", realid.toAscii().data(), level);
	}
}

int APNHelper::apnPowerSave(QString apid){
	QString realid = apid.replace(" ", "@32@");
	GConfItem item( (QString(GCONF_APN_LIST) + "/") + realid + GCONF_APN_POWER);
	bool ok; int ret;
	ret = item.value(GCONF_APN_POWER_NOTFOUND).toInt(&ok);
	if(! ok)
		ret = GCONF_APN_POWER_INVALID;
	if(ret == GCONF_APN_POWER_NOTFOUND)
		qDebug("The power management of %s is undefined", realid.toAscii().data());
	else if(ret == GCONF_APN_POWER_INVALID)
		qDebug("The power management of %s is invalid", realid.toAscii().data());
	else
		qDebug("The power management of %s is %d", realid.toAscii().data(), ret);
	return ret;
}
