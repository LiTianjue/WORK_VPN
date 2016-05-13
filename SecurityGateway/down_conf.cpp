#include "down_conf.h"
#include <QFile>
#include <QMessageBox>
#include<QtDebug>
#include "myhelper.h"
#include "FaceInfoMan.h"
#include <QByteArray>


CDOWN_CONF::CDOWN_CONF(QObject *parent)
{
    timeOut = new QTimer(this);
    connect(timeOut,SIGNAL(timeout()),this,SLOT(handleTimeOut()));

    qEvent= new QEventLoop();

    tcpClient = new QTcpSocket(this);
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(ReadData()));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),
            this,SLOT(ReadError(QAbstractSocket::SocketError)));

    _rt.clear();
    err_string.clear();
    
    xml = new CHandle_XML();
}

CDOWN_CONF::~CDOWN_CONF()
{
    _rt.clear();

    delete timeOut;
    delete qEvent;
    tcpClient->abort();
    delete tcpClient;
    
    delete xml;
}

bool CDOWN_CONF::TryConnect(QString IP, int port)
{
    tcpClient->abort();	//取消原有连接

    tcpClient->connectToHost(IP,port);
    if(tcpClient->waitForConnected(1000))
    {
        //QMessageBox::information(this,tr("服务连接"),tr("连接认证服务器成功"),tr("确定"));
        return true;

    }else
    {
        err_string = tcpClient->errorString();
        return false;
    }
}

QByteArray &CDOWN_CONF::GetResponse(const QString &_request)
{
    _rt.clear();

    tcpClient->waitForBytesWritten(1000);
    if(tcpClient->isWritable())
        tcpClient->write(_request.toLocal8Bit());
        //tcpClient->write(_request);
    else
        return _rt;


    timeOut->start(REQ_TIMEOUT);
    qEvent->exec();

    return _rt;
}

QByteArray &CDOWN_CONF::GetResponse(QByteArray &_request)
{
    _rt.clear();

    tcpClient->waitForBytesWritten(1000);
    if(tcpClient->isWritable())
        tcpClient->write(_request);
        //tcpClient->write(_request);
    else
        return _rt;


    timeOut->start(REQ_TIMEOUT);
    qEvent->exec();

    return _rt;
}

bool CDOWN_CONF::SaveConf(QString Path)
{
    return SaveFile(Path,_rt);
}

bool CDOWN_CONF::CheckResponse(QString Node, QString OK_String)
{
    QString RespString;
    RespString = QString(xml->GetNode(Node));

    if(RespString == OK_String)
    {
        return true;
    }

    return false;
}

bool CDOWN_CONF::Save_xml(QString perfixPath, QString nameNode, QString sizeNode, QString contextNode)
{
    QString name = xml->GetNode(nameNode).simplified();
    qDebug()<<"[" << name<<"]"<<endl;
    int size = xml->GetNode(sizeNode).simplified().toInt();
    //QByteArray context = xml->GetNode(contextNode);
    QByteArray context = QByteArray::fromBase64(xml->GetNode(contextNode));
    qDebug()<<"[" << context<<"]"<<endl;
    qDebug()<<"SIZE:" << context.size() <<endl;

    QString Path = perfixPath + name;
    return myHelper::SaveFile(Path,context);
}

QString CDOWN_CONF::Save_xml_tmp(QString tmp_dir, QString nameNode, QString sizeNode, QString contextNode)
{
    QString name = xml->GetNode(nameNode).simplified();
    qDebug()<<"[" << name<<"]"<<endl;
    int size = xml->GetNode(sizeNode).simplified().toInt();
    //QByteArray context = xml->GetNode(contextNode);
    QByteArray context = QByteArray::fromBase64(xml->GetNode(contextNode));
    qDebug()<<"[" << context<<"]"<<endl;
    qDebug()<<"SIZE:" << context.size() <<endl;

    //QString Path = "\"" +tmp_dir + name + "\"";
    QString Path =tmp_dir + name;
    if(myHelper::SaveFile(Path,context))
        return Path;
    return NULL;
}


bool CDOWN_CONF::SaveFile(QString Path, QByteArray &text)
{
    QFile::remove(Path);
    QFile *file;
    file = new QFile(Path);
    if(!file->open(QIODevice::WriteOnly)){
        delete file;
        err_string = file->errorString();
        return false;
    }
    //QString hhh = text;
    if(file)
        file->write(text.data());
    file->flush();
    file->close();

    delete  file;
    return true;
}

void CDOWN_CONF::ReadData()
{
    _rt.clear();

    int len = ReadHeader();
    qDebug()<<"can read " << len << " data";
    if(len > 0)
    {
        while(_rt.size() < len)
        {
            _rt += tcpClient->readAll();
            qDebug() << "Total read "<<_rt.size() <<" data";
            if(_rt.size() < len )
            {
                if(tcpClient->waitForReadyRead(1000))
                    continue;
                else
                    break;
            }else
            {
                break;
            }

        }
    }


    //qDebug()<<_rt<<endl;
    qDebug()<<"_rt size:"<<_rt.size();
    if(_rt.isEmpty())
    {
        //QMessageBox::(this,"测试","什么也没读到");
    }
    timeOut->stop();
    qEvent->exit();
    tcpClient->disconnectFromHost();
}

void CDOWN_CONF::ReadError(QAbstractSocket::SocketError)
{
    tcpClient->disconnectFromHost();

    timeOut->stop();
    qEvent->exit();

    err_string = tcpClient->errorString();
}

void CDOWN_CONF::handleTimeOut()
{
    timeOut->stop();
    qEvent->exit();
    err_string = "服务器响应超时";
    tcpClient->disconnectFromHost();
}

int CDOWN_CONF::ReadHeader()
{
    char header[PROTOCOL_HEADER_LEN] = {0};
    int context_len = 0;
    int ret = 0;
    ret = tcpClient->read(header,PROTOCOL_HEADER_LEN);
    //qDebug()<<"client read ret = "<<ret <<endl;
    ret = check_netheader2(header,&context_len);

    //qDebug()<<"check ret  = "<<ret <<endl;
    //qDebug()<<"context len = "<<context_len <<endl;

    if(ret == 1)
        return context_len;
    else
        return 0;
}
//构建请求正文
QByteArray CDOWN_CONF::BuildContext_1(char *data, int len)
{
    /*
    <?xml version="1.0"?>
    <Down>
    <Certificate>
    </Certificate>
    <CertificateName>client.der</CertificateName>
    <CertificateSize>1460</CertificateSize>
    <CmdType>DownConfig</CmdType>
    </Down>
    */
    QByteArray certContext = QByteArray(data,len);
    //char *perfix = "<?xml version=\"1.0\"?>\r\n<Down>\r\n<C>";

    char perfix[1024] = {0};
    char postfix[1024] = {0};

    sprintf(perfix,"%s%s%s%s%s%s",
            "<?xml version=\"1.0\"?>\r\n",
            "<Down>\r\n",
            "<OsType>\r\n",
            "Windows\r\n",
            "</OsType>\r\n",
            "<Certificate>\r\n");

    sprintf(postfix,"%s%s%s%s%s%d%s%s%s%s%s",
            "</Certificate>\r\n",
            "<CertificateName>\r\n",
            "client.der\r\n",
            "</CertificateName>\r\n",
            "<CertificateSize>",
            len,
            "</CertificateSize>",
            "<CmdType>\r\n",
            "DownConfig\r\n",
            "</CmdType>\r\n",
            "</Down>");

    QByteArray context =QByteArray(perfix,strlen(perfix)) + certContext + QByteArray(postfix,strlen(postfix));
    qDebug()<<"per "<<strlen(perfix)<<" post "<<strlen(postfix) << endl;
    qDebug() <<"cert len:"<< certContext.size() <<endl;
     return context;

}

QByteArray CDOWN_CONF::BuildContext_2(QByteArray base64_data, int len)
{
    //QByteArray certContext = QByteArray(data,len);
    //char *perfix = "<?xml version=\"1.0\"?>\r\n<Down>\r\n<C>";

    char perfix[1024] = {0};
    char postfix[1024] = {0};

    sprintf(perfix,"%s%s%s%s%s%s",
            "<?xml version=\"1.0\"?>\r\n",
            "<Down>\r\n",
            "<OsType>\r\n",
            "Windows\r\n",
            "</OsType>\r\n",
            "<Certificate>\r\n");

    sprintf(postfix,"\r\n%s%s%s%s%s%d%s%s%s%s%s",
            "</Certificate>\r\n",
            "<CertificateName>\r\n",
            "client.der\r\n",
            "</CertificateName>\r\n",
            "<CertificateSize>",
            len,
            "</CertificateSize>",
            "<CmdType>\r\n",
            "DownConfig\r\n",
            "</CmdType>\r\n",
            "</Down>");


    QByteArray context =QByteArray(perfix,strlen(perfix)) + base64_data + QByteArray(postfix,strlen(postfix));
    //QByteArray context =QByteArray(perfix,strlen(perfix))+QByteArray(postfix,strlen(postfix));
    //qDebug()<<"per "<<strlen(perfix)<<" post "<<strlen(postfix) << endl;
    //qDebug() <<"cert len:"<< certContext.size() <<endl;
     return context;
}

QByteArray CDOWN_CONF::BuildReq(QByteArray context)
{
   if(context.isEmpty())
       return NULL;
   char header[PROTOCOL_HEADER_LEN];  // L 0x4C Z 0x5A
   build_netheader2(header,context.size());
   QByteArray req = QByteArray(header,PROTOCOL_HEADER_LEN) + context;
   return req;

}
