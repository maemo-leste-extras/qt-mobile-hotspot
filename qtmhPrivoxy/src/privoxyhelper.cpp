#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QStringListIterator>
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include "../../qtMobileHotspot/src/mobilehotspotconfiguration.h"
#include "../../qtMobileHotspot/src/commandhelper.h"
#include "pluginconfiguration.h"
#include "privoxyhelper.h"


PrivoxyHelper::PrivoxyHelper(){
}

PrivoxyHelper::~PrivoxyHelper(){
}

int PrivoxyHelper::generateConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf){
	/* 0=ok, -1=fail */
	qDebug("Generating Privoxy configuration file %s from %s", CONFIGURATION_FILE, CONFIGURATION_FILE_QTMH);
	int ret;
	QString cmd("cat ");
	cmd += (QString(CONFIGURATION_FILE_QTMH) + " > ") + CONFIGURATION_FILE;
	CommandHelper::executeCommandOld(cmd, &ret);
	if(ret != 0){
		qDebug("FAILURE : Cannot copy our privoxy sample configuration");
		return -1;
	}

	cmd = ((QString("echo \"listen-address ") + configuration->lanNetwork) + "1:") + QString::number(conf->port) + "\"";
	cmd += QString(" >> ") + CONFIGURATION_FILE;
	CommandHelper::executeCommandOld(cmd, &ret);
	if(ret != 0){
		qDebug("FAILURE : Cannot append listen-address to the privoxy configuration file");
		return -1;
	}
	if(conf->defaultActions){
		cmd = QString("echo \"actionsfile match-all.action\" >> ") + CONFIGURATION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append match-all.action to the privoxy configuration file");
			return -1;
		}
		cmd = QString("echo \"actionsfile default.action\" >> ") + CONFIGURATION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append default.action to the privoxy configuration file");
			return -1;
		}
	}
	if(conf->forward){
		cmd = QString("echo \"forward / ") + conf->forwardIP; cmd += ":"; cmd += QString::number(conf->forwardPort) + "\" >> ";
		cmd += CONFIGURATION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append forwarding to the privoxy configuration file");
			return -1;
		}
	}
	return 0;
}

int PrivoxyHelper::deleteConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf){
	/* 0=ok, -1=fail */
	qDebug("Deleting Privoxy configuration file %s", CONFIGURATION_FILE);
	QFile file(CONFIGURATION_FILE);
	if(file.exists())
		return file.remove() ? 0 : 1;
	return 0;
}

int PrivoxyHelper::generateActionFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf){
	/* 0=ok, -1=fail */
	qDebug("Generating Privoxy action file %s from %s", ACTION_FILE, ACTION_FILE_QTMH);
	int ret;
	QString cmd("cat ");
	cmd += (QString(ACTION_FILE_QTMH) + " > ") + ACTION_FILE;
	CommandHelper::executeCommandOld(cmd, &ret);
	if(ret != 0){
		qDebug("FAILURE : Cannot copy our privoxy action file");
		return -1;
	}

	if(conf->ua != UA_NOCHANGE){
		cmd = QString("echo \"{ +hide-user-agent{");
		if(conf->ua == UA_MICROB)
			cmd += MICROB;
		else if(conf->ua == UA_IPHONE)
			cmd += IPHONE;
		else
			cmd += conf->customUA;
		cmd += "} }";
		cmd += QString("\" >> ") + ACTION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append our settings to the privoxy configuration file");
			return -1;
		}
		cmd = QString("echo \"/ # All websites\" >> ") + ACTION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append our settings to the privoxy configuration file");
			return -1;
		}
	}
	return 0;
}

int PrivoxyHelper::deleteActionFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf){
	/* 0=ok, -1=fail */
	qDebug("Deleting Privoxy action file %s", ACTION_FILE);
	QFile file(ACTION_FILE);
	if(file.exists())
		return file.remove() ? 0 : 1;
	return 0;
}

int PrivoxyHelper::runPrivoxy(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process){
	/* 0=ok, -1=fail */
	qDebug("Running Privoxy %s", PRIVOXY_FILE);
	QString cmd = (QString(PRIVOXY_FILE) + " --no-daemon ") + CONFIGURATION_FILE;
	bool ret = CommandHelper::executeCommandAsync(cmd, process, 5000);
	return ret ? 0 : -1;
}

int PrivoxyHelper::stopPrivoxy(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process){
	/* 0=ok, -1=fail */
	qDebug("Terminating Privoxy");
	process->terminate();
	return process->waitForFinished(5000) ? 0 : 1;
}

int PrivoxyHelper::setIptables(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf){
	/* 0=ok, -1=fail */
	qDebug("Setting iptables for Privoxy");
	QStringList commands; // $IPTABLES -t nat -A PREROUTING -i $LANIF -d ! $LANNET -p tcp --dport $PORT -j REDIRECT --to-port 8118
	QString baseCmd("/usr/sbin/iptables -t nat -A PREROUTING");
	baseCmd += QString(" -i ") + configuration->lanInterface;
	baseCmd += (QString(" -d ! ") + configuration->lanNetwork) + "0/24";
	baseCmd += " -p tcp --dport";

	QStringList ports;
	ports = conf->ports.split(",");
	QStringListIterator iterator(ports);
	while(iterator.hasNext()){
		QString port = iterator.next().trimmed();
		QString cmd = (baseCmd + QString(" ")) + port;
		cmd += QString(" -j REDIRECT --to-port ") + QString::number(conf->port);
		commands.append(cmd);
	}

	iterator = QStringListIterator(commands);
	while(iterator.hasNext()){
		QString cmd = iterator.next();
		int ret;
		bool res = CommandHelper::executeCommand(cmd, &ret);
		if(ret != 0 || res == false){
			qDebug("FAILURE : Last command generated an error - exiting iptables for privoxy");
			return -1;
		}
	}
	return 0;
}

