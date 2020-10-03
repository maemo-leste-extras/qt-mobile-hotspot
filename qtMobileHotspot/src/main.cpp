#include <QtGui/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QString>
#include <unistd.h>
#include "commandhelper.h"
#include "mobilehotspotgui.h"


int main(int argc, char *argv[])
{
	if(getuid() != 0){
			qDebug("Not running as root, lets try to run another instance as root");
			QString run("./util/start.sh");
			int ret; CommandHelper::executeCommandOld(run, &ret);
			qDebug("Other instance finished, ending");
			return ret;
	}

    QApplication app(argc, argv);
    QApplication::setApplicationName("Qt Mobile Hotspot");
    QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));

    MobileHotspotGUI gui;
    gui.show();

    return app.exec();
}
