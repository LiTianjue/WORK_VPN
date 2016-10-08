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
#ifdef MANAGMENT
#include "ManagerServer/managerserver.h"
#endif /*END MANAGMENT*/

#include "myhelper.h"

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
#ifdef MANAGMENT
        //QMessageBox::warning(NULL,("安全网关服务"),("已经有安全网关服务在运行在运行"),("确定"));
        //int msgret =  myHelper::ShowMessageBoxQuesion("已经有安全网关服务在运行，是否停止服务？");
        if(myHelper::ShowMessageBoxQuesionX("已经有安全网关服务在运行，是否停止服务？") == QMessageBox::Yes)
        {
            //QMessageBox::warning(NULL,("安全网关"),("关闭安全网关设备"),("确定"));
            QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",QSettings::NativeFormat);
            reg->setValue("ProxyEnable",false);

            QProcess *poc = new QProcess;
            poc->execute("kill_vpnserver.bat");


            delete poc;
            delete reg;

        }
        else
        {
            //QMessageBox::warning(NULL,("安全网关"),("保持运行"),("确定"));
        }
#else
        QMessageBox::warning(NULL,("安全网关"),("已经有客户端在运行"),("确定"));
#endif
        qApp->quit();
        return ret;
    }

    MainWindow w;


    //w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    //w.setWindowFlags(Qt::WindowStaysOnTopHint);
    //w.setFixedSize(400,300);
    w.setFixedSize(500,370);
#ifdef MANAGMENT
    //要先检查一下证书并且证书需要被记住
    //输入并验证pin码
    
    MultiClientServer *MtServer= NULL;
    MtServer = new MultiClientServer();
    MtServer->start(MANAGMENT_PORT);
    MtServer->mwind = &w;
    MtServer->mwind->hide();
    //MtServer->mwind->show();

    if(!w.cmd_perconnect())
    {
        QMessageBox::warning(NULL,("安全网关"),("安全网关启动失败"),("确定"));
        qApp->quit();
        return ret;
    }
    //QMessageBox::information(NULL,("安全网关"),("安全网关已启动，请稍后查看启动状态"),("确定"));
#else
    w.show();
#endif

    return a.exec();
}

