# This is the qtMobileHotspot global qmake file
# qtMobileHotspot is divided between the application and its plugins
# We are actually in /workspace/qtmobilehotspot
# Application is in /workspace/qtmobilehotspot/qtMobileHotspot
# Plugins are in /workspace/qtmobilehotspot/<plugin_name>
# Debian packaging info is in /workspace/qtmobilehotspot/debian
# Note about the extras-assistant : Copy the qtmobilehotspot folder in workspace, rename it to qtmobilehotspot-version, and rename the esbox project (contained in .project) to qtmobilehotspot-version
# Note about the extras-assistant #2 : Package with 'dpkg-buildpackage -rfakeroot -sa -S -i -us -uc' in order to avoid 'public key not found'


# QMAKE version enforcement
QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT4 ) {
error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

# We will compile subdirs
TEMPLATE = subdirs

# First we compile the application itself, which is in /workspace/qtmobilehotspot/qtMobileHotspot (so ./qtMobileHotspot)
# Then we have some plugins to make, in /workspace/qtmobilehotspot/<plugin_name> (so ./<plugin_name>)
SUBDIRS = ./qtMobileHotspot \
          ./qtmhDummy \
          ./qtmhNetFix \
          ./qtmhIptablesFix \
          ./qtmhMacFilter \
          ./qtmhDNSMasq \
          ./qtmhPrivoxy
