#include "mainwindow.h"
#include <QApplication>
#include <QtCore>
#include<QSharedMemory>
#include<QMessageBox>
#include<windows.h>
#include<QtNetwork/QLocalSocket>
#include<QtNetwork/QLocalServer>
#include <QtDebug>
#include <QFile>
#include <QTextStream>

/*----Change Log-----------------------------*/
/*  	Data		Author			Desc
 * 	2015-07-07		LiTianjue	添加忘记证书按钮，配置文件不存在从模板创建，
 * 	2015-07-07		LiTianjue	点击启动按钮会先检查ca.crt和ta.key文件是否存在
 * 	2015-07-07		LiTianjue	修改MyGet.exe下载配置文件，重新读取修改，版本升级为2.1
 *
 *  2015-07-13		LiTianjue	使用QTcpSocket检测服务器联通性，不再使用外挂MyGet.exe
 *  2015-07-13		LiTianjue	添加文件http.cpp ,http.h下载文件（写死），不再使用MyGet.exe外挂下载
 *  2015-07-13		LiTianjue	增加下载失败提示
 *
 *
 *--------------------------------------------*/

int main(int argc, char *argv[])
{
    int ret = 0;
    QApplication a(argc, argv);
        //QMessageBox::warning(NULL,("安全网关"),("已经有客户端在运行"),("确定"));

    QString serverName = QCoreApplication::applicationName();
    //QString serverName = "SecurityGateway";
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(500)) { //如果能够连接得上的话，将参数发送到服务器，然后退出
        QTextStream stream(&socket);
        QStringList args = QCoreApplication::arguments();
        if (args.count() > 1)
            stream << args.last();
        else
            stream << QString();
        stream.flush();
        socket.waitForBytesWritten();
        QMessageBox::warning(NULL,("安全网关"),("已经有客户端在运行"),("确定"));
        qApp->quit();
        return ret;
    }

    MainWindow w;

    //w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    //w.setWindowFlags(Qt::WindowStaysOnTopHint);
    //w.setFixedSize(400,300);
    w.setFixedSize(500,370);
    w.show();

    return a.exec();
}

