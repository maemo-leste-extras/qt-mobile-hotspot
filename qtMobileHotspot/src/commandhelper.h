#ifndef COMMANDHELPER_H
#define COMMANDHELPER_H

#include <QtCore/QString>
#include <QtCore/QProcess>



class CommandHelper
{

public:
	CommandHelper();
    ~CommandHelper();
    static int  executeCommandOld(QString command);
	static void executeCommandOld(QString command, int *result);
    static bool executeCommand(QString command, int timeoutStartedMS = 10000, int timeoutFinishedMS = -1);
    static bool executeCommand(QString command, int *result, int timeoutStartedMS = 10000, int timeoutFinishedMS = -1);
    static bool executeCommandWithStdOut(QString command, int *result, QString *stdout, int timeoutStartedMS = 10000, int timeoutFinishedMS = -1);
    static bool executeCommandWithStdOutErr(QString command, int *result, QString *stdout, QString *stderr, int timeoutStartedMS = 10000, int timeoutFinishedMS = -1);
    static bool executeCommandAsync(QString command, QProcess *process, int timeoutStartedMS = 10000);
};

#endif // COMMANDHELPER_H
