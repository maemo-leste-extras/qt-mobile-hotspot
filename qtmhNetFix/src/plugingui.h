#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>
#include "ui_plugingui.h"
#include "pluginconfiguration.h"


class PluginGUI : public QMainWindow
{
    Q_OBJECT

public:
    PluginGUI(QWidget *parent = 0);
    ~PluginGUI();

    void configuration(PluginConfiguration *conf);
    void setConfiguration(PluginConfiguration &conf);

protected:
	void closeEvent(QCloseEvent* event);

private:
    Ui::PluginGUIClass ui;

signals:
	void windowClosed();
};

#endif // PLUGINGUI_H
