#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QStringListIterator>
#include <QtCore/QDir>
#include "mobilehotspotconfiguration.h"
#include "languagegui.h"

LanguageGUI::LanguageGUI(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	ui.lstLanguages->addItem(tr("EN (default)"), DEFAULT_LANGUAGE);

	QDir dir("./lang");
	QStringList namefilters;
	namefilters.append("*.qtmh.qm");
	QStringList list = dir.entryList(namefilters, QDir::Files | QDir::NoSymLinks);
	QStringListIterator iterator(list);
	while(iterator.hasNext()){
		QString filename = iterator.next();
		QString code = filename.replace(".qtmh.qm", "");
		QString displayed = code.toUpper();
		ui.lstLanguages->addItem(displayed, code);
	}
}

LanguageGUI::~LanguageGUI()
{

}

void LanguageGUI::setConfiguration(MobileHotspotConfiguration &configuration){
	int index = 0;
	for(int i = 0; i < ui.lstLanguages->count(); i++)
		if(ui.lstLanguages->itemData(i).toString() == configuration.language)
			index = i;
	ui.lstLanguages->setCurrentIndex(index);
}

void LanguageGUI::configuration(MobileHotspotConfiguration *configuration){
	configuration->language = ui.lstLanguages->itemData(ui.lstLanguages->currentIndex()).toString();
}
