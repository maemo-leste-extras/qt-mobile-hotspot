#ifndef LANGUAGEGUI_H
#define LANGUAGEGUI_H

#include <QtWidgets/QDialog>
#include "mobilehotspotconfiguration.h"
#include "ui_languagegui.h"

class LanguageGUI : public QDialog
{
    Q_OBJECT

public:
    LanguageGUI(QWidget *parent = 0);
    ~LanguageGUI();

    void setConfiguration(MobileHotspotConfiguration &configuration);
    void configuration(MobileHotspotConfiguration *configuration);

private:
    Ui::LanguageGUIClass ui;
};

#endif // LANGUAGEGUI_H
