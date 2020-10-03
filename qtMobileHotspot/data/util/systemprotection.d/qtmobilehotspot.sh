#!/bin/sh
echo "We are now in the protection script for the package qtmobilehotspot"
failed="no"

echo "We are checking active internet connections"
gprsenabled="no"
wlanenabled="no"
val=`dbus-send --print-reply --system --dest=com.nokia.icd2 /com/nokia/icd2 com.nokia.icd2.state_req | grep "uint32 0" | wc -l`
if [ "$val" == "0" ]; then
	echo "System reports an active internet connection, trying to guess if it is WLAN or GPRS"
	val=`route | grep gprs0 | wc -l`
	if [ "$val" == "0" ]; then
		echo "GPRS seems to be disabled (no route linked to gprs0), it must be WLAN"	
		wlanenabled="yes"
	else
		echo "GPRS seems to be enabled (a route to gprs0 exists)"
		gprsenabled="yes"
	fi	
else
	echo "System reports no active internet connection, considering GPRS and WLAN disabled"
fi

echo "We are now going to check if WLAN driver wl12xx is loaded"
val=`lsmod | grep wl12xx | wc -l`
if [ "$val" == "0" ]; then
	echo "GOTCHA : WLAN driver wl12xx isn't loaded, loading it"
	modprobe wl12xx
	failed="yes"
else
	echo "OK : Test passed"
fi

echo "We are now going to check WLAN networking (must be in Managed mode)"
val=`iwconfig wlan0 | grep "Mode:Managed" | wc -l`
if [ "$val" == "0" ]; then
	echo "GOTCHA : Wifi mode isn't Managed, resetting it"
	ifconfig wlan0 down
	iwconfig wlan0 mode managed
	ifconfig wlan0 up
	failed="yes"
else
	echo "OK : Test passed"
fi

if [ "$wlanenabled" == "yes" ]; then
	echo "We are now going to check WLAN networking (must be up)"
	val=`ifconfig | grep "wlan0" | wc -l`
	if [ "$val" == "0" ]; then
		echo "GOTCHA : Wifi interface is down, enabling it"
		ifconfig wlan0 up
		failed="yes"
	else
		echo "OK : Test passed"
	fi
else
	echo "Skipping the WLAN networking test (must-be-up) because WLAN is not active"
fi

echo "We are now going to check USB modules (g_ether must not be loaded)"
val=`lsmod | grep g_ether | wc -l`
if [ "$val" == "1" ]; then
	echo "GOTCHA : g_ether module loaded, we'll unload it"
	rmmod g_ether
	failed="yes"
else
	echo "OK : Test passed"
fi

echo "We are now going to check USB modules (one of g_nokia or g_file_storage must be loaded)"
passed="no"
val=`lsmod | grep g_nokia | wc -l`
[ "$val" == "1" ] && passed="yes"
val=`lsmod | grep g_file_storage | wc -l`
[ "$val" == "1" ] && passed="yes"
if [ "$passed" == "no" ]; then
	echo "GOTCHA : No USB module loaded, we'll fix this by loading g_file_storage"
	modprobe g_file_storage
	failed="yes"
else
	echo "OK : Test passed"
fi

echo "We are now going to check for remaining DNSMasq running (should be none)"
val=`ps | grep "dnsmasq --no-daemon -i " | grep -v grep | wc -l`
if [ "$val" == "1" ]; then	
	failed="yes"
	val=`ps | grep "dnsmasq --no-daemon -i " | grep -v grep | awk -F " " '{print $1}'`
	echo "GOTCHA : DNSMasq seems to be running, we are going to kill it (process is $val)"
	kill -9 $val
else
	echo "OK : Test passed"
fi

echo "We are now going to check for remaining Privoxy running (should be none)"
val=`pidof "privoxy.executable" | wc -l`
if [ "$val" == "1" ]; then	
	failed="yes"
	val=`pidof "privoxy.executable"`
	echo "GOTCHA : Privoxy seems to be running, we are going to kill it (process is $val)"
	kill -9 $val
else
	echo "OK : Test passed"
fi
	
echo "System protection for qtmobilehotspot finished, returning according to previous tests"
[ "$failed" == "yes" ] && exit 1
exit 0

