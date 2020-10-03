#ifndef ACCESSPOINTGUI_H
#define ACCESSPOINTGUI_H

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>
#include "mobilehotspotconfiguration.h"
#include "ui_accesspointgui.h"

class AccessPointGUI : public QMainWindow
{
    Q_OBJECT

public:
    AccessPointGUI(QWidget *parent = 0);
    ~AccessPointGUI();
    void setConfiguration(MobileHotspotConfiguration &configuration);
    void configuration(MobileHotspotConfiguration *configuration);

protected:
	void closeEvent(QCloseEvent* event);

private:
    Ui::AccessPointGUIClass ui;
    QString validKey();
    QString validMAC();

signals:
	void windowClosed();
};

#endif // ACCESSPOINTGUI_H
