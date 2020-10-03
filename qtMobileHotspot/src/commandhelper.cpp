#include <QtCore/QString>
#include <QtCore/QProcess>
#include "commandhelper.h"


CommandHelper::CommandHelper(){
}

CommandHelper::~CommandHelper(){
}

bool CommandHelper::executeCommand(QString command, int timeoutStartedMS, int timeoutFinishedMS){
	/* true=ok, false=fail */
	int result;
	return executeCommand(command, &result, timeoutStartedMS, timeoutFinishedMS);
}

bool CommandHelper::executeCommand(QString command, int *result, int timeoutStartedMS, int timeoutFinishedMS){
	/* true=ok, false=fail */
	QString stdout;
	return executeCommandWithStdOut(command, result, &stdout, timeoutStartedMS, timeoutFinishedMS);
}

bool CommandHelper::executeCommandWithStdOut(QString command,  int *result, QString *stdout, int timeoutStartedMS, int timeoutFinishedMS){
	/* true=ok, false=fail */
	QString stderr;
	return executeCommandWithStdOutErr(command, result, stdout, &stderr, timeoutStartedMS, timeoutFinishedMS);
}

bool CommandHelper::executeCommandWithStdOutErr(QString command, int *result, QString *stdout, QString *stderr, int timeoutStartedMS, int timeoutFinishedMS){
	/* true=ok, false=fail */
	qDebug( (QString("Command : ") + command).toAscii().data() );
	QProcess process;
	process.start(command);
	bool ret = process.waitForStarted(timeoutStartedMS);
	if(! ret){
		qDebug( (QString("FAILURE : Command was too slow to start (") + process.errorString() + ")").toAscii().data() );
		return false;
	}
	ret = process.waitForFinished(timeoutFinishedMS);
	if(! ret){
		qDebug( (QString("FAILURE : Command was too slow to end (") + process.errorString() + ")").toAscii().data() );
		return false;
	}
	*stdout = QString(process.readAllStandardOutput());
	*stderr = QString(process.readAllStandardError());
	*result = process.exitCode();
	return true;
}

int CommandHelper::executeCommandOld(QString command){
	int result;
	executeCommandOld(command, &result);
	return result;
}

void CommandHelper::executeCommandOld(QString command, int *result){
	qDebug( (QString("Old-Command : ") + command).toAscii().data() );
	int ret = system(command.toAscii().data());
	*result = WEXITSTATUS(ret);
}

bool CommandHelper::executeCommandAsync(QString command, QProcess *process, int timeoutStartedMS){
	/* true=ok, false=fail */
	qDebug( (QString("Async-Command : ") + command).toAscii().data() );
	if(process->state() != QProcess::NotRunning){
		qDebug("FAILURE : the given QProcess is linked to a running process, please terminate it");
		return -1;
	}
	process->start(command);
	if(! process->waitForStarted(timeoutStartedMS)){
		qDebug((QString("FAILURE : Async-Command did not run on time (") + process->errorString() + ")").toAscii().data());
		return false;
	}
	return true;
}
