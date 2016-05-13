#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QSettings>
//#include <QtNetwork>
#include<QtNetwork/QLocalSocket>
#include<QtNetwork/QLocalServer>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include <windows.h>
#include <locale>
#include <QDebug>
#include <dbt.h>

#include "tconfigfile.h"
#include <windows.h>

#include "down_conf.h"
#include "public_def.h"
#include "simple_log.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QProcess* vpnprocess;

protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);


private slots:
    //quit form tray,kill openvpn first
    void quitfrom_tray();
    //add for vpn proocess
    void start_process();
    void start_read_output();
    void start_read_err_output();
    void finish_process(int exitCode, QProcess::ExitStatus exitStatus);

    void newLocalSocketConnection();

    void on_pushButton_Login_clicked();

    void on_pushButton_Test_clicked();

    void on_pushButton_ConfigApply_clicked();

    void on_pushButton_ProxyApply_clicked();

    void on_pushButton_ProxyCancel_clicked();

    void on_pushButton_ConfigCancel_clicked();

    void on_pushButton_disconnection_clicked();

    void on_pushButton_back_clicked();

    void on_check_autoRun_clicked();


    void on_checkBox_thumb_clicked();

    void on_check_proxy_clicked();

private:
    Ui::MainWindow *ui;

private:
    //处理openvpn的连接状态的
    void WatchOpenVPNProcess();
    void CheckMsg(QString msg);
    bool IsMsg(QString msg,QString key);
    void printMessage(QString msg);
    QString TransforChinese(QString msg);


    //add for trayIcon
    QAction *restoreAction;
    QAction *quitAction;
    void createAction();
    void createTrayIcon();
    void setIcon();
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    //Checkfor only
    QLocalServer *m_localServer;

     //QTcpSocket *tcpsock;
     //add for config file
     TConfigFile *m_pConfigFile;	/*ovpn配置文件类*/
     //主配置文件类 m_Conf.ini
     QSettings *Con_Ini;

     QSettings *download_Ini;
     QString __replace_server_IP(const QString &strSourceURL);
     QString __replace_download_server_IP(const QString &strSourceURL);

     void Conf_2_UI();
     void UI_2_Conf();
     int DownloadCert();
     void AutoRun(bool bAuto);		/*设置开机自启动*/

     //显示配置信息
     void ShowConf();
     void ProxySetting();

 //add by andy 2015-12-02 增加证书验证和配置下载
     CDOWN_CONF *down;
     bool per_connect();
  //add by andy 20115-12-03 增加一个类保装参数
     VpnParams *vpn_params;
     //void set_vpn_params();

protected:
    bool nativeEvent(const QByteArray & eventType, void * message, long*result);
private:
    QTemporaryDir tmpdir;
private :
    void ReConfigOvpn(QString ofile);

    void StopVpn();
};

#endif // MAINWINDOW_H
