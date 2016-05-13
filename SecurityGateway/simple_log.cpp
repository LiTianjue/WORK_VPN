
#include "simple_log.h"

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QString>
#include <QTime>

static QFile LogFile;
static QTextStream ts;

void MessageOutput(QtMsgType type , const QMessageLogContext &context , const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString log;
    switch(type)
    {
    case QtDebugMsg:
        //log = QString("Debug: ");
        break;


    case QtWarningMsg:
        log = QString("Error: ");
        break;


    case QtCriticalMsg:
        log = QString("Critical Error: ");
        break;


    case QtFatalMsg:
        log = QString("Fatal Error: ");
        abort();
        break;


    default:
        log = QString("Unknow Msg Type : ");
        break;
    };

#ifdef LOCAL_DEBUG
    log += QTime::currentTime().toString("hh:mm:ss.zzz, File: ") + QString(context.file) ;
    log += QString(", Line: " ) + QString::number(context.line) + QString(" : %1\n").arg(msg);
#else
    log += QTime::currentTime().toString("hh:mm:ss.zzz: ") + QString("%1\n").arg(msg);
#endif
    ts << log;
    ts.flush();


    mutex.unlock();
}

void Log_on(QString file)
{
    if(LogFile.isOpen())
        LogFile.close();

    LogFile.setFileName(file);
    LogFile.open(QIODevice::WriteOnly | QIODevice::Append);
    ts.setDevice(&LogFile);

    qInstallMessageHandler(MessageOutput);
}

void Log_off()
{
    if(LogFile.isOpen())
        LogFile.close();

    qInstallMessageHandler(0);
}
