#ifndef PLUGINCONFIGURATION_H
#define PLUGINCONFIGURATION_H

#include <QtCore/QStringList>
#include <QtCore/QString>
#include "../../qtMobileHotspot/src/mobilehotspotpluginhelper.h"

#define DEFAULT_IPTABLES_CHAINFORWARD "-N IPTABLESFIX_F"
#define DEFAULT_IPTABLES_CHAININPUT   "-N IPTABLESFIX_I"
#define DEFAULT_IPTABLES_I_IPSPOOF "-A IPTABLESFIX_I -i $LANIF$ -s $LANIP$ -d ! $LANBROAD$ -j DROP"
#define DEFAULT_IPTABLES_F_IPSPOOF "-A IPTABLESFIX_F -i $LANIF$ -s $LANIP$ -j DROP"
#define DEFAULT_IPTABLES_I_MACSPOOF "-A IPTABLESFIX_I -i $LANIF$ -m mac --mac-source $LANMAC$ -d ! $LANBROAD$ -j DROP"
#define DEFAULT_IPTABLES_F_MACSPOOF "-A IPTABLESFIX_F -i $LANIF$ -m mac --mac-source $LANMAC$ -j DROP"
#define DEFAULT_IPTABLES_ADDFORWARD "-A FORWARD -j IPTABLESFIX_F"
#define DEFAULT_IPTABLES_ADDINPUT "-A INPUT -j IPTABLESFIX_I"
#define DEFAULT_IPTABLES DEFAULT_IPTABLES_CHAINFORWARD "\n" DEFAULT_IPTABLES_CHAININPUT "\n" DEFAULT_IPTABLES_F_IPSPOOF "\n" DEFAULT_IPTABLES_I_IPSPOOF "\n" DEFAULT_IPTABLES_F_MACSPOOF "\n" DEFAULT_IPTABLES_I_MACSPOOF "\n" DEFAULT_IPTABLES_ADDFORWARD "\n" DEFAULT_IPTABLES_ADDINPUT


class PluginConfiguration{

public:
	PluginConfiguration(MobileHotspotPluginHelper *helper);
	~PluginConfiguration();
	void save();
	void load();
	void reset();

	QStringList list;
	static const QString defaultIptables;

private:
	MobileHotspotPluginHelper *helper; // Helper for this plugin
};


#endif // PLUGINCONFIGURATION_H

