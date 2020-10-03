#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QStringListIterator>
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include "../../qtMobileHotspot/src/mobilehotspotconfiguration.h"
#include "../../qtMobileHotspot/src/commandhelper.h"
#include "pluginconfiguration.h"
#include "dnsmasqhelper.h"


DNSMasqHelper::DNSMasqHelper(){
}

DNSMasqHelper::~DNSMasqHelper(){
}

int DNSMasqHelper::generateConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf){
	/* 0=ok, -1=fail */
	qDebug("Generating DNSMasq configuration file %s from %s", CONFIGURATION_FILE, CONFIGURATION_FILE_QTMH);
	int ret;
	QString cmd("cat ");
	cmd += QString(CONFIGURATION_FILE_QTMH) + " > " + CONFIGURATION_FILE;
	CommandHelper::executeCommandOld(cmd, &ret);
	if(ret != 0){
		qDebug("FAILURE : Cannot copy our dnsmasq sample configuration");
		return -1;
	}

	QString text = QString("dhcp-range=") + configuration->lanNetwork + conf->dhcpFrom + "," + configuration->lanNetwork + conf->dhcpTo;
	cmd = QString("echo \"") + text + "\"";
	cmd += QString(" >> ") + CONFIGURATION_FILE;
	CommandHelper::executeCommandOld(cmd, &ret);
	if(ret != 0){
		qDebug("FAILURE : Cannot append dhcp range to the dnsmasq configuration file");
		return -1;
	}

	if(conf->noHosts){
		cmd = "echo \"no-hosts\" ";
		cmd += QString(" >> ") + CONFIGURATION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append no-hosts to the dnsmasq configuration file");
			return -1;
		}
	}

	if(conf->domainName != ""){
		text = QString("domain=") + conf->domainName;
		cmd = QString("echo \"") + text + "\"";
		cmd += QString(" >> ") + CONFIGURATION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append domain name to the dnsmasq configuration file");
			return -1;
		}
	}

	QStringListIterator iterator(conf->dnsTable);
	while(iterator.hasNext()){
		QStringList entry = iterator.next().split("/");
		if(entry.size() != 2){
			qDebug("FAILURE : Invalid entry in DNS table");
			return -1;
		}
		QString domain = entry.at(0);
		domain = domain.replace("$ALL$", "#");
		QString ip = entry.at(1);
		ip = ip.replace("$LANIP$", configuration->lanNetwork + "1");
		ip = ip.replace("$FILTER$", "127.0.0.1");
		text = QString("address=/") + domain + "/" + ip;
		cmd = QString("echo \"") + text + "\"";
		cmd += QString(" >> ") + CONFIGURATION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append dns entry to the dnsmasq configuration file");
			return -1;
		}
	}

	iterator = QStringListIterator(conf->dhcpTable);
	while(iterator.hasNext()){
		QStringList entry = iterator.next().split("/");
		if(entry.size() != 3){
			qDebug("FAILURE : Invalid entry in DHCP table");
			return -1;
		}
		QString hostname = entry.at(0);
		QString mac = entry.at(1);
		QString ip = configuration->lanNetwork + entry.at(2);
		text = "dhcp-host=";
		if(hostname == "")
			text += mac;
		else if(mac == "")
			text += hostname;
		else
			text += mac + "," + hostname;
		text += QString(",") + ip;
		cmd = QString("echo \"") + text + "\"";
		cmd += QString(" >> ") + CONFIGURATION_FILE;
		CommandHelper::executeCommandOld(cmd, &ret);
		if(ret != 0){
			qDebug("FAILURE : Cannot append dhcp entry to the dnsmasq configuration file");
			return -1;
		}
	}

	return 0;
}

int DNSMasqHelper::deleteConfigurationFile(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf){
	/* 0=ok, -1=fail */
	qDebug("Deleting DNSMasq configuration file %s", CONFIGURATION_FILE);
	QFile file(CONFIGURATION_FILE);
	if(file.exists())
		return file.remove() ? 0 : 1;
	return 0;
}

int DNSMasqHelper::runDNSMasq(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process){
	/* 0=ok, -1=fail */
	qDebug("Running DNSMasq");
	QString cmd("/usr/sbin/dnsmasq");
	cmd += " -i "; cmd += configuration->lanInterface; // listen on lan interface
	cmd += " -a "; cmd += configuration->lanNetwork + "1"; // listen on our address
	cmd += " -I lo"; // do not listen on loopback
	cmd += " -z"; // bind only to selected interfaces
	if(configuration->internetEnabled){
		cmd += " --dhcp-option=3,";
		cmd += configuration->lanNetwork + "1"; // dhcp option 3 = router
	}
	cmd += " --dhcp-option=6,"; cmd += configuration->lanNetwork + "1"; // dhcp option 6 = dns server (we give it even if DNS is disabled, as some systems may dislike not knowing any DNS server)
	cmd += " --conf-file="; cmd += CONFIGURATION_FILE; // our configuration file
	bool ret = CommandHelper::executeCommandAsync(cmd, process, 5000);
	return ret ? 0 : -1;
}

int DNSMasqHelper::stopDNSMasq(const MobileHotspotConfiguration *configuration, PluginConfiguration *conf, QProcess *process){
	/* 0=ok, -1=fail */
	qDebug("Terminating DNSMasq");
	process->terminate();
	return process->waitForFinished(5000) ? 0 : 1;
}
