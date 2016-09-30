#include "managerserver.h"
#include <QTcpSocket>
#include <QString>

#ifdef MANAGMENT
MultiClientServer::MultiClientServer()
{
    connect(this, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}
MultiClientServer::~MultiClientServer()
{
    stop();
}
void MultiClientServer::stop()
{
    qDeleteAll(clientConnections);
}
void MultiClientServer::start(int port)
{
    listen(QHostAddress::LocalHost,port);
}

void MultiClientServer::handleNewConnection()
{
    while (hasPendingConnections()) {
            QTcpSocket *client = nextPendingConnection();
            connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
            connect(client,SIGNAL(readyRead()),this,SLOT(readyRead()));
            clientConnections.append(client);
            //sendHello(client);
        }
}

void MultiClientServer::readyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
       if (!client)
           return;

       if (client->bytesAvailable()) {
           //QByteArray length;
           //QString receivelength=client->read(4);
           QByteArray readData = client->readAll();
           //quint32 length= receivelength.toUInt();
           int length = readData.length();
           qDebug()<<length;
           //QString receiveMsg=client->read(length);
           //qDebug()<<receiveMsg;
           qDebug()<<readData;
           HandleMsg(readData,client);

       }
}

void MultiClientServer::clientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());

       if (!client)
           return;

       clientConnections.removeAll(client);
       client->deleteLater();
}

void MultiClientServer::sendHello(QTcpSocket *client)
{
    if (!client)
        return;

    client->write("Hello\n");
}



void MultiClientServer::HandleMsg(QByteArray msg, QTcpSocket *client)
{
    QString rtMsg="";
    if(msg.contains("VPN_UP"))
    {
        qDebug() << "HANDLE VPN UP ";
        mwind->cmd_start();
        rtMsg+="VPN_OK";
    }
    else if(msg.contains("VPN_DOWN"))
    {
        qDebug() << "HANDLE VPN DOWN ";
        mwind->cmd_stop();
        rtMsg+="VPN_OK";

    }
    else if(msg.contains("VPN_STATUS")) {
        qDebug() << "HANDLE VPN STATUS";
        rtMsg+=mwind->cmd_satus();

    }
    else if(msg.contains("VPN_LOG"))
    {
        qDebug() << "HANDLE VPN LOG";
        //rtMsg+=mwind->cmd_log();
        rtMsg+="VPN_OK";
    }
    else if(msg.contains("VPN_EXIT"))
    {
        qDebug() << "HANDLE VPN EXIT";
        mwind->cmd_exit();
    }
    else
    {
        qDebug() << "HANDLE Unknow Cmd";
        rtMsg+="VPN_UNKNOW";
    }

    if(!client)
        return;
    /*
    if(!client->waitForBytesWritten(1000))
        return;
    */
    client->waitForBytesWritten(1000);
    if(client->isWritable())
        client->write(rtMsg.toLatin1());

}



#endif /*END MANAGMENT*/
