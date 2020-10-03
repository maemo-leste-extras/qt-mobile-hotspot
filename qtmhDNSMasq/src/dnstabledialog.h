#ifndef DNSTABLEDIALOG_H
#define DNSTABLEDIALOG_H

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>
#include <QtCore/QStringList>
#include "pluginconfiguration.h"
#include "ui_dnstabledialog.h"

class DNSTableDialog : public QMainWindow
{
    Q_OBJECT

public:
	DNSTableDialog(QWidget *parent = 0);
    ~DNSTableDialog();

void configuration(QStringList *conf);
void setConfiguration(QStringList &conf);

protected:
	void closeEvent(QCloseEvent* event);

private:
    Ui::DNSTableDialogClass ui;

    void tableToList(QStringList *list);
    void listToTable(QStringList list);

private slots:
	void loadDNSTable();
	void saveDNSTable();
	void addDNS();
	void removeDNS();

signals:
	void windowClosed();
};

#endif // DNSTABLEDIALOG_H
