#!/bin/sh
echo "============================================================"
echo "This is Qt Mobile Hotspot launcher, starting the program now"
echo "============================================================"
cd /opt/qtmobilehotspot
echo "run-standalone.sh ./qtmobilehotspot" | sudo gainroot
echo "======================================================="
echo "We are now going to start the system protection utility"
cd /opt/qtmobilehotspot/util
echo "./systemprotection.sh" | sudo gainroot
echo "========================"
echo "Everything finished, bye"

