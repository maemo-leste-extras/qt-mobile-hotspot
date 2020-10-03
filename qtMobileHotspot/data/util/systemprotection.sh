#!/bin/sh
echo "============================================================="
echo "This is system protection for Qt Mobile Hotspot"
echo "We will try to ensure that nothing bad happens to your system"
protectionfailed="no"

echo "Saving WLAN MAC in case we have to cycle the WLAN driver"
wlanMAC=`ifconfig wlan0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'`

echo "Using .sh scripts located in /opt/qtmobilehotspot/util/systemprotection.d"
for script in `find ./systemprotection.d/ | grep ".sh"` ; do
	echo "---------------------------------------"
	echo "Script : $script"
	$script
	res="$?"
	echo "---------------------------------------"
	if [ "$res" != "0" ]; then
		echo "** GOTCHA : The script detected a problem **"
		protectionfailed="yes"
	fi
done

if [ "$protectionfailed" == "yes" ]; then
	echo "** At least one script encountered a problem, performing a connectivity reset **"
	echo "Clearing IPTables and ipforward"
	echo 0 > /proc/sys/net/ipv4/ip_forward
	iptables -F
	iptables -t nat -F
	iptables -t mangle -F
	iptables -X
	iptables -t nat -X
	iptables -t mangle -X
	iptables -P INPUT ACCEPT
	iptables -P OUTPUT ACCEPT
	iptables -P FORWARD ACCEPT	
	echo "Sending a disconnect request via DBus (for resetting ICD)"
	dbus-send --print-reply --system --dest=com.nokia.icd2 /com/nokia/icd2 com.nokia.icd2.disconnect_req uint32:0x8000 >/dev/null	
	echo "Unloading and reloading the Wifi driver (try to keep the WLAN MAC also)"
	rmmod wl12xx
	modprobe wl12xx
	sleep 1
	ifconfig wlan0 down
	ifconfig wlan0 hw ether $wlanMAC
	echo "Printing a message to the user"
	dbus-send --system --type=method_call --dest=org.freedesktop.Notifications /org/freedesktop/Notifications org.freedesktop.Notifications.SystemNoteInfoprint string:"Some problems due to a possible bad termination of Qt Mobile Hotspot have been fixed"
else
	echo "No problem encountered, we are fine" 
fi

echo "System protection for Qt Mobile Hotspot terminating now"

