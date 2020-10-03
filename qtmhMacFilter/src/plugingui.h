#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>
#include <QtGui/QCheckBox>
#include <QtCore/QString>
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
    QCheckBox *lastClickedMac;

    void removeAllMACs();
    bool isValidMAC(QString name, QString mac);

signals:
	void windowClosed();

private slots:
	void addMac();
	void removeMac();
	void enableAll();
	void disableAll();
	void macClicked();
	void load();
	void save();
};

#endif // PLUGINGUI_H
