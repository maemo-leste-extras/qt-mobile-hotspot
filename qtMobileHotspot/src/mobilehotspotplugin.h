#ifndef MOBILEHOTSPOTGUIPLUGIN_H
#define MOBILEHOTSPOTGUIPLUGIN_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QMainWindow>
#include <QtGui/QIcon>
#include <QtCore/QTranslator>
#include "mobilehotspotconfiguration.h"

#define PROVIDE_SYSTEMMODULES "System Modules"
#define PROVIDE_USBMODULE "USB Module"
#define PROVIDE_USBINTERFACE "USB Interface"
#define PROVIDE_WLANINTERFACE "WLAN Interface"
#define PROVIDE_DNSDHCP "DNS/DHCP"
#define PROVIDE_IPTABLES "Iptables"


class MobileHotspotPlugin{

public:
	virtual QString name() = 0;
	virtual int order() = 0;
	virtual QStringList provided() = 0;

	virtual QString displayedName() = 0;
	virtual QString displayedAuthor() = 0;
	virtual QString displayedContact() = 0;
	virtual QString displayedDescription() = 0;
	virtual QIcon displayedIcon() = 0;

	virtual bool allowConfigureStopped() = 0;
	virtual bool allowConfigureRunning() = 0;
	virtual bool allowEnableDisableStopped() = 0;
	virtual bool allowEnableDisableRunning() = 0;

	virtual bool isPluginEnabled() = 0;
	virtual void setPluginEnabled(bool enabled, const MobileHotspotConfiguration *configuration, bool running) = 0;

	virtual void configure(const QMainWindow *parentWindow, const MobileHotspotConfiguration *configuration, bool running, bool reset) = 0;
	virtual bool provide(QString service, const MobileHotspotConfiguration *configuration) = 0;
	virtual bool unprovide(QString service, const MobileHotspotConfiguration *configuration, bool successStarting) = 0;
	virtual void beforeStarting(const MobileHotspotConfiguration *configuration) = 0;
	virtual void afterStarting(const MobileHotspotConfiguration *configuration, bool successStarting) = 0;
	virtual void beforeStopping(const MobileHotspotConfiguration *configuration, bool successStarting) = 0;
	virtual void afterStopping(const MobileHotspotConfiguration *configuration, bool successStarting, bool successFinishing) = 0;

	virtual QTranslator *translator() = 0;

};

Q_DECLARE_INTERFACE(MobileHotspotPlugin, "org.dest.qtmh.plugin/0.1.3")


#endif // MOBILEHOTSPOTGUIPLUGIN_H
