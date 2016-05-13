#ifndef HTTP
#define HTTP
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>

#include <QTimer>
#include <QByteArray>
#include <QThread>
#include <QEventLoop>
#include <QtNetwork/QNetworkReply>
#include <QTcpSocket>
#include <QProgressDialog>
#include <QFile>

class Http:public QObject
{
        Q_OBJECT
public:
    Http(QObject *parent = 0);
    ~Http();
    QByteArray& GetDownLoadUrl(const QString& _url);
    QByteArray& GetDownLoadUrl(const QString& _url,int timeout);
    QByteArray& GetDownLoadUrl(QUrl url,int timeout);
    void SaveFile(QString Path,QByteArray& text);

    //下载进度条

    bool DownloadFile(QString filename, QString down_url, int filesize);
    QProgressDialog process;
    QFile savefile;

public slots:
    void replyFinished(QNetworkReply*); /* download finished */
    void slotError(QNetworkReply::NetworkError); /* handle error */
    void handleTimeOut(); /* handle time out */
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    QNetworkAccessManager    *downloadManager;
    QEventLoop*   qEvent;
    QTimer        *timeOut;
    QByteArray _rt;
    QNetworkReply* reply;

public:
    QString err_string;
};
#endif // HTTP

