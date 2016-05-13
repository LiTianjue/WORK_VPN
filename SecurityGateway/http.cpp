#include "http.h"
#include <QFile>
#include <QMessageBox>
#include <QUrlQuery>


Http::Http(QObject *parent)
{
    downloadManager = new QNetworkAccessManager(this);
    connect(downloadManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    timeOut = new QTimer(this);
    connect(timeOut, SIGNAL(timeout()), this, SLOT(handleTimeOut()));
    qEvent=new QEventLoop();
    reply=NULL;

}

Http::~Http()
{
    _rt.clear();
    if(savefile.isOpen())
        savefile.close();
    delete downloadManager;
    delete timeOut;
    delete qEvent;
}

QByteArray& Http::GetDownLoadUrl(const QString &_url)
{
    _rt.clear();
    QUrl url(_url);

    if (!url.isValid()) return _rt;

    QNetworkRequest* request=new QNetworkRequest(url);
    reply = downloadManager->get(*request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

    timeOut->start(2000);
    qEvent->exec();
    reply->close();
    delete reply;
    delete request;
    return _rt;
}

QByteArray &Http::GetDownLoadUrl(const QString &_url, int timeout)
{
    _rt.clear();
    QUrl url(_url);

    if (!url.isValid()) return _rt;

    QNetworkRequest* request=new QNetworkRequest(url);
    reply = downloadManager->get(*request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

    timeOut->start(timeout);
    qEvent->exec();
    reply->close();
    delete reply;
    delete request;
    return _rt;

}

QByteArray &Http::GetDownLoadUrl(QUrl url, int timeout)
{
    _rt.clear();

    if (!url.isValid()) return _rt;

    QNetworkRequest* request=new QNetworkRequest(url);
    reply = downloadManager->get(*request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));

    timeOut->start(timeout);
    qEvent->exec();
    reply->close();
    delete reply;
    delete request;
    return _rt;

}

void Http::SaveFile(QString Path, QByteArray &text)
{
   QFile::remove(Path);

   QFile *file;
   file = new QFile(Path);
   if(!file->open(QIODevice::WriteOnly)){
      // QMessageBox::information(this,"Config","Config Error");
       delete file;
       return ;
   }
   QString hhh= text;
   if(file)
       //file->write(text);
       file->write(text.data());
   file->flush();
   file->close();

   delete file;
   return ;
}

//下载并保存文件到本地
bool Http::DownloadFile(QString filename, QString down_url,int filesize)
{
    QUrl url(down_url);
    err_string.clear();

    if (!url.isValid())
    {
        err_string = "错误的下载地址";
        return false;
    }

    savefile.setFileName(filename);
    if(!savefile.open(QIODevice::WriteOnly))
    {
        err_string = "文件 "+filename+" 打开失败" ;
        return false;
    }

    process.setLabelText(tr("更新下载包中"));
    process.setRange(0,filesize);
    process.setModal(true);
    process.setCancelButtonText(tr("取消"));
    process.setValue(0);
    process.show();

    QNetworkRequest* request=new QNetworkRequest(url);
    reply = downloadManager->get(*request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));


    //下载的时候设置5分钟超时
    timeOut->start(1000*60*5);
    qEvent->exec();
    reply->close();
    delete reply;
    delete request;
    //saved.close();
    if(err_string.isEmpty())
        return true;
    else
        return false;

}

void Http::replyFinished(QNetworkReply *)
{
    qDebug()<<"Finished";
    _rt.append( reply->readAll() );

    timeOut->stop();
    qEvent->exit();
}

void Http::slotError(QNetworkReply::NetworkError)
{
    err_string = "请求错误";
    timeOut->stop();
    qEvent->exit();
}

void Http::handleTimeOut()
{
    err_string = "请求超时";
    timeOut->stop();
    qEvent->exit();
}

void Http::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    /*
    qDebug()<<"Progess";
    //add for test
    QFile *file;
    file = new QFile("clent.ovpn");
    if(!file->open(QIODevice::WriteOnly)){
      // QMessageBox::information(this,"Config","Config Error");
       delete file;
       return ;
   }else{
    file->write(reply->readAll()) ;
    file->flush();
    file->close();
    delete file;
   }
   */

    /*
    QString line;
    do{
        line.clear();
        line = reply->readLine();
        _rt.append(line);
        _rt.append( "\r");
    }while(!line.isEmpty());

    //reply->readline())
    //_rt.append( reply->readLine());
    //_rt.append( "\r\n");
    qDebug()<<_rt;

    */
    if(savefile.isOpen())
    {
        if(process.wasCanceled())
        {
            qDebug() << "下载被取消";
            timeOut->stop();
            qEvent->exit();
            err_string = "下载被取消，老版本无法登陆,请及时更新";
            return;
        }
        process.setValue(bytesReceived);
        savefile.write(reply->readAll());

    }else
    {
        _rt.append( reply->readAll() );
        _rt.append( reply->readAll() );
    }
}





