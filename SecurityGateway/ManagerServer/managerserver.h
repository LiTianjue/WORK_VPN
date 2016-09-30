#ifndef MULTICLIENTSERVER_H
#define MULTICLIENTSERVER_H
#include <QTcpServer>
#include "mainwindow.h"

#define MANAGMENT_PORT 9914

class MultiClientServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit MultiClientServer();
    ~MultiClientServer();

    void start(int port);
    void stop();

protected:
    void sendHello(QTcpSocket *client);

protected slots:
    void handleNewConnection();
    void clientDisconnected();
    void readyRead();
    //bool hasPendingConnection();

private:
    QList<QTcpSocket *> clientConnections;

public:
    MainWindow *mwind;
private:
    void HandleMsg(QByteArray msg ,QTcpSocket *client);
};



#endif // MULTICLIENTSERVER_H
