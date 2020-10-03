#ifndef PLUGINSCONTROLGUI_H
#define PLUGINSCONTROLGUI_H

#include <QMainWindow>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QListWidgetItem>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QList>
#include "mobilehotspotconfiguration.h"
#include "mobilehotspotplugin.h"
#include "ui_pluginscontrolgui.h"


class PluginsControlGUI : public QMainWindow
{
    Q_OBJECT

public:
    PluginsControlGUI(QWidget *parent = 0);
    ~PluginsControlGUI();

    void setConfiguration(MobileHotspotConfiguration *configuration, bool running, QMap<QString, MobileHotspotPlugin*> *plugins, QList<QString> *sortedPlugins);

protected:
	void closeEvent(QCloseEvent* event);

private:
    Ui::PluginsControlGUIClass ui;
    MobileHotspotConfiguration *configuration;
    QMap<QString, MobileHotspotPlugin*> *plugins;
    QList<QString> *sortedPlugins;
    bool running;

private slots:
	void pluginSelected(QListWidgetItem *current, QListWidgetItem *previous);
	void configurePlugin();
	void enableDisablePlugin();
	void resetPlugin();

signals:
	void windowClosed();
};

#endif // PLUGINSCONTROLGUI_H
