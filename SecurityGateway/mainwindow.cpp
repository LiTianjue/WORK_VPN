#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "http.h"

#include <QtCore>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QMessageBox>
#include <QtNetwork>
#include<QtNetwork/QLocalSocket>
#include <QDebug>
#include<QtNetwork/QLocalServer>
#include "public_def.h"

#include <QtDebug>
#include <QFile>
#include <QTextStream>

#include <shellapi.h>

#include "down_conf.h"
#include "myhelper.h"
#include "win32_csp.h"
#include "handle_json.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //check a
    QString serverName = QCoreApplication::applicationName();
#if 0
    qDebug()<<serverName;
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
        return;
    }
#endif
    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()),this, SLOT(newLocalSocketConnection())); //监听新到来的连接
    if (!m_localServer->listen(serverName)) {
        if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
                        && QFile::exists(m_localServer->serverName())) { //确保能够监听成功
            QFile::remove(m_localServer->serverName());
            m_localServer->listen(serverName);
            QMessageBox::warning(NULL,("安全网关"),("监听失败"),("确定"));
    }
    }

#if 1
    /*********************************************/
    //注册usb插拔检测
    /*********************************************/
    static const GUID GUID_DEVINTERFACE_LIST[] =
    {
    // GUID_DEVINTERFACE_USB_DEVICE
    { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
    // GUID_DEVINTERFACE_DISK
    { 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
    // GUID_DEVINTERFACE_HID,
    { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
    // GUID_NDIS_LAN_CLASS
    { 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }
    //// GUID_DEVINTERFACE_COMPORT
    //{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },
    //// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },
    //// GUID_DEVINTERFACE_PARALLEL
    //{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },
    //// GUID_DEVINTERFACE_PARCLASS
    //{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
    };

    //注册插拔事件
    HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotifacationFiler;
    ZeroMemory(&NotifacationFiler,sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    NotifacationFiler.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotifacationFiler.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    for(int i=0;i<sizeof(GUID_DEVINTERFACE_LIST)/sizeof(GUID);i++)
    {
        NotifacationFiler.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];//GetCurrentUSBGUID();//m_usb->GetDriverGUID();

        hDevNotify = RegisterDeviceNotification((HANDLE)this->winId(),&NotifacationFiler,DEVICE_NOTIFY_WINDOW_HANDLE);
        if(!hDevNotify)
        {
            int Err = GetLastError();
            qDebug() << "注册失败" <<endl;
            //MessageBox("RegisterDeviceNotificationFailed");
        }
        //else
        //	MessageBox("RegisterDeviceNotificationSuccess");
    }
#endif
/**********************************************************************************/


    ui->setupUi(this);
    //set taryIcon
    createAction();
    createTrayIcon();
    setIcon();
    trayIcon->show();

    //设置ip和端口号的校验
    QRegExp ipRe("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-4]|[01]?\\d\\d?)");
    QRegExpValidator *pIpValidator = new QRegExpValidator(ipRe,this);
    ui->lineEdit_server_ip->setValidator(pIpValidator);
    ui->proxy_ip->setValidator(pIpValidator);
    //限制端口号范围
    ui->lineEdit_server_port->setValidator(new QIntValidator(0,65535,this));
    ui->lineEdit_strategy_port->setValidator(new QIntValidator(0,65535,this));
    ui->proxy_port->setValidator(new QIntValidator(0,65535,this));


    //配置文件相关的代码
    if(!QFile::exists((QCoreApplication::applicationDirPath() +"/m_Config.ini")))
    {
        //QMessageBox::warning(this,tr("配置检查"),tr("配置文件不存在，请检查安装是否正确！"),MessageBox::tr("确定"));
        QMessageBox::warning(this,tr("配置检查"),tr("配置文件不存在，请检查安装是否正确！"),tr("确定"));
        return;
    }
#ifdef CHECK_OVPN
    //打开配置文件读取数据
    QString vpn_conf =((QCoreApplication::applicationDirPath()));
    qDebug()<<"运行路径："<<vpn_conf<<endl;
    //int nPos = vpn_conf.indexOf("bin");
    int nPos = vpn_conf.indexOf("debug");
    vpn_conf = vpn_conf.mid(0,nPos);
    //qDebug()<< vpn_conf;
    vpn_conf += "/config/client.ovpn";
    vpn_conf = vpn_conf.replace("//","/");
    //qDebug()<< "vpn_conf"<< vpn_conf;

    if(!QFile::exists(vpn_conf))
    {
        //QMessageBox::warning(this,tr("配置检查"),tr("VPN 配置文件不存在，请检查安装是否正确！"));
        //写入一个模板文件 add by LiTianjue 2015-07-07
        QFile file(vpn_conf);
        if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::warning(this,tr("配置检查"),tr("VPN 配置文件不存在，请检查安装是否正确！"));
            return;
        }
        //QMessageBox::warning(this,tr("配置检查"),tr("VPN 配置文件丢失，使用默认模板！"));
        QTextStream out(&file);
        out.setCodec("UTF-8"); //请注意这行

        out<<"client"<<endl;
        out<<"max-routes 5000"<<endl;
        out<<"dev tun"<<endl;
        out<<"proto tcp"<<endl;
        out<<"remote 192.168.1.212 1194"<<endl;
        out<<"resolv-retry infinite"<<endl;
        out<< "persist-key"<<endl;
        out<< "persist-tun"<<endl;
        out<< "ca ca.crt"<<endl;
        out<< "tls-auth ta.key 1"<<endl;
        out<< "cipher DES-EDE3-CBC"<<endl;
        out<< "comp-lzo"<<endl;
        out<< "verb 3"<<endl;

        out.flush();
        file.close();
    }
    m_pConfigFile = new TConfigFile(vpn_conf);

#endif
    Con_Ini = new QSettings((QCoreApplication::applicationDirPath() +"/m_Config.ini"),QSettings::IniFormat);
    //qDebug()<< "server ip:"<<Con_Ini->value("/setup/server_ip").toString();


    //检查证书下载配置文件 2015-12-01 不需要http 下载了
#if 0
    if(!QFile::exists((QCoreApplication::applicationDirPath() +"/m_download_cfg.ini")))
    {
        QMessageBox::warning(this,tr("配置检查"),tr("配置文件缺失，请检查安装是否正确！"),tr("确定"));
    }
    else
    {
      download_Ini = new QSettings((QCoreApplication::applicationDirPath() +"/m_download_cfg.ini"),QSettings::IniFormat);
      //原代码中读取配置显示在界面上但是界面上没有对应的文本框，这里我们就不做这个操作了
    }
#endif
    Conf_2_UI();

// add for check and download by andy 2015-12-02
    down = new CDOWN_CONF(this);
// add for process by andy 2015-12-03
    vpn_params = new VpnParams(this);

// add for test tmpdir 2015-12-08
    if(tmpdir.isValid())
    {
        qDebug()<<"tmp dir is :" << tmpdir.path();
    }

    //读取配置文件，获取一些初始化的选项
    if(Con_Ini->value("common/remember_thumb").toString() == "yes")
    {
        vpn_params->remember_thumb = true;
        vpn_params->thumb = Con_Ini->value("common/thumb").toString();
    }else{
        vpn_params->remember_thumb = false;
    }

    if(Con_Ini->value("common/debug").toString() == "__on")
    {
         Log_on("debug.log");
         vpn_params->debug = true;
        //开启调试，内部测试用
    }
    vpn_params->current_version = Con_Ini->value("version/current_version").toString();

    vpn_params->check_version = Con_Ini->value("per_connect/check_version").toString();
    vpn_params->down_exe = Con_Ini->value("per_connect/down_exe").toString();
    vpn_params->verify = Con_Ini->value("per_connect/verify").toString();

    ui->label_7->setText("安全网关客户端Ver"+vpn_params->current_version);
    vpn_params->os_type = myHelper::GetSysVersion();

    vpn_params->config_md5 = Con_Ini->value("others/config_md5").toString();

    //SetAppPath
    vpn_params->work_path = QCoreApplication::applicationDirPath();
    qDebug() << "Work Path" << vpn_params->work_path;
    vpn_params->conf_dir = vpn_params->work_path;
    vpn_params->log_file = vpn_params->work_path + "/debug.log";
    vpn_params->exe = vpn_params->work_path +"/openvpn.exe";
    vpn_params->tmp_dir = tmpdir.path();
}


MainWindow::~MainWindow()
{
    delete ui;
    if(Con_Ini)
        delete Con_Ini;
// add for check and download by andy 2015-12-02
    if(down)
        delete down;
// add for check and download by andy 2015-12-03
    if(vpn_params)
        delete vpn_params;
}


/*************************************
 * Function : Windows behavies
 * ***********************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    //hide();
    event->ignore();
}


/*****************************************
 * Function : Tray
 *
 *****************************************/
//add for tary
void MainWindow::createAction()
{
    restoreAction = new QAction(tr("主界面"),this);
    connect(restoreAction,SIGNAL(triggered()),this,SLOT(showNormal()));
    //quitAction = new QAction(tr("&Exit"),this);
    quitAction = new QAction(tr("退出"),this);
    connect(quitAction,SIGNAL(triggered()),this,SLOT(quitfrom_tray()));


    return ;
}

void MainWindow::quitfrom_tray()
{
    qDebug("click exit");
    //干掉openvpn

    QProcess *poc = new QProcess;
    poc->execute("kill_openvpn.bat");

    QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",QSettings::NativeFormat);

    reg->setValue("ProxyEnable",false);

    //delete poc;
    qApp->quit();

}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    //connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::DoubleClick())), this, SLOT(showNormal()));
}

void MainWindow::setIcon()
{
    QIcon icon(":/disconnection.png");
    trayIcon->setIcon(icon);
    setWindowIcon((icon));
    trayIcon->setToolTip(tr(" 安全网关"));
    trayIcon->show();
    return;
}

/***************************************
 *  Function : Configure
 ***************************************/
void MainWindow::Conf_2_UI()
{
    //读取配置显示在界面上
    ui->lineEdit_server_ip->setText(Con_Ini->value("setup/server_ip").toString());
    ui->lineEdit_server_port->setText(Con_Ini->value("setup/server_port").toString());
    ui->lineEdit_strategy_port->setText(Con_Ini->value("setup/strategy_server_port").toString());

    //if((m_pConfigFile->m_strProto.toLower()).trimmed() == "tcp")
    if((Con_Ini->value("common/proto").toString().simplified() == "tcp"))
    {
        ui->ProtoBox->setCurrentIndex(0);
    }
    else
    //else if((m_pConfigFile->m_strProto.toLower()) == tr("udp"))
    {
        ui->ProtoBox->setCurrentIndex(1);
    }

    //ui->lineEdit_server_ip->setText(m_pConfigFile->m_strVpn_server_ip);
    //ui->lineEdit_server_port->setText(m_pConfigFile->m_strVpn_server_port);

    //auto run
    QString strAuto = Con_Ini->value("setup/openvpn_exe_auto").toString().simplified();
    if(strAuto == "off")
    {
      //qDebug()<<"equl to off";
        ui->check_autoRun->setChecked(false);
    }
    else if(strAuto == "on")
    {
        //qDebug()<<"equl to on";
        ui->check_autoRun->setChecked(true);
    }

    //proxy setting
    QString strAutoSetting = Con_Ini->value("proxy_setup/proxy_auto_setting").toString();
    if(strAutoSetting == "off")
    {
        ui->check_proxy->setChecked(false);
    }
    else if(strAutoSetting == "on")
    {
        ui->check_proxy->setChecked(true);
    }
    ui->proxy_ip->setText(Con_Ini->value("proxy_setup/proxy_server_ip").toString());
    ui->proxy_port->setText(Con_Ini->value("proxy_setup/proxy_server_port").toString());

    //这里还有新增负载均很的配置没有写
    //新增的选项 记住证书
    if(Con_Ini->value("common/remember_thumb").toString() == "yes")
    {
        ui->checkBox_thumb->setChecked(true);
    }
    else
    {
        ui->checkBox_thumb->setChecked(false);
    }
}


void MainWindow::UI_2_Conf()
{
    //保存VPN配置
    QString ip = ui->lineEdit_server_ip->text();
    QString port = ui->lineEdit_server_port->text();
#ifdef CHECK_OVPN
    if(m_pConfigFile)
    {
        //保存到VPN配置文件
        QString proto = ui->ProtoBox->currentText().toLower();
        //QMessageBox::about(NULL,"test",proto.toLower());
        m_pConfigFile->m_strProto = proto;
        QString ca = m_pConfigFile->m_strCa;
        m_pConfigFile->SaveCfgFile(proto,ip,port,ca);
    }
#endif
    //保存到m_Config.ini
    Con_Ini->setValue("setup/server_ip",ip);
    Con_Ini->setValue("setup/server_port",port);
    Con_Ini->setValue("setup/strategy_server_port",ui->lineEdit_strategy_port->text());
    //是不是要同步一下才能写到文件？

    //ProxySetting() 另一个界面上处理

    //保存自动运行到m_Config.ini

    if(ui->check_autoRun->checkState() == 2)
    {
        Con_Ini->setValue("setup/openvpn_exe_auto","on");
        //AutoRun(true);
    }
    else
    {
        //AutoRun(false);
        Con_Ini->setValue("setup/openvpn_exe_auto","off");
    }

    //在m_Config 文件中，使用服务器IP替换下载URL中的IP
    //不再使用http下载
#if 0
    QString strSendURL = "http://222.46.20.17:1195/ClientAccess_access.action";
    Con_Ini->setValue("setup/send_url",__replace_server_IP(strSendURL));
    Con_Ini->setValue("setup/tscc_cms_url",__replace_server_IP(strSendURL));
    //往m_download_cfg.ini文件写入
    //原代码界面中没有输入的控件，所以这部分其实是写死的
    QString strDownloadURL =  download_Ini->value("setup1/url").toString();
    download_Ini->setValue("setup1/url",__replace_download_server_IP(strDownloadURL));
    download_Ini->setValue("setup1/local","ca.crt");
    strDownloadURL =  download_Ini->value("setup2/url").toString();
    download_Ini->setValue("setup2/url",__replace_download_server_IP(strDownloadURL));
    download_Ini->setValue("setup2/local","client.ovpn");
    strDownloadURL =  download_Ini->value("setup3/url").toString();
    download_Ini->setValue("setup3/url",__replace_download_server_IP(strDownloadURL));
    download_Ini->setValue("setup3/local","ta.key");


#endif
    ///
    //最重要的一环，需要设置证书自动下载
    //SetDownloadClientAutoRun();
    //检查服务器地址和端口是否发生变化，如果发生变化需要马上下载证书
    //if()
    //RestarServices()
    //SendCopyData()

    QString proto = ui->ProtoBox->currentText();
    Con_Ini->setValue("common/proto",proto);
}


/***********************************************************
 * Function : DownloadCert
 * *******************************************************/
int MainWindow::DownloadCert()
{
    QString save_path =((QCoreApplication::applicationDirPath()));
    int nPos = save_path.indexOf("bin");
    save_path = save_path.mid(0,nPos);
    //qDebug()<< vpn_conf;
    save_path += "/config/";
    save_path = save_path.replace("//","/");

    Http* http = new Http();
    //qDebug()<<ui->lineEdit->text();
    QString url_perfix = "http://"+ui->lineEdit_server_ip->text()+":"+ui->lineEdit_strategy_port->text();
    QString ca_postfix = "/ClientAction_downCa.action";
    QString ta_postfix = "/ClientAction_downStaticKey.action";
    QString ovpn_postfix = "/ClientAction_downWindowsConfig.action";
#if 1
    QByteArray& array1=http->GetDownLoadUrl(url_perfix+ca_postfix);
    if(array1.isEmpty())
    {
        QMessageBox::warning(this,tr("保存配置"),tr("保存CA失败"),tr("确定"));
        delete http;
        return -1;
    }else{
        http->SaveFile(save_path+"ca.crt",array1);
    }
    //ui->textBrowser->append(array1);

    QByteArray& array2=http->GetDownLoadUrl(url_perfix+ta_postfix);
    if(array2.isEmpty())
    {
        QMessageBox::warning(this,tr("保存配置"),tr("保存静态秘钥失败"),tr("确定"));
        delete http;
        return -1;
    }
    //ui->textBrowser->append(array2);
    http->SaveFile(save_path+"ta.key",array2);
#endif
    QByteArray& array3=http->GetDownLoadUrl(url_perfix+ovpn_postfix);
    if(array3.isEmpty())
    {
        QMessageBox::warning(this,tr("保存配置"),tr("下载配置文件失败"),tr("确定"));
        delete http;
        return -1;
    }
    //ui->textBrowser->append(array3);
    http->SaveFile(save_path+"client.ovpn",array3);
    delete http;
    return 0;


#if 0
    QString exe = "MyGet -d";
    QProcess *downloadCert = new QProcess;
    //downloadCert->start(exe);
    downloadCert->execute(exe);
    /*
    if(false == downloadCert->waitForStarted())
    {
        QMessageBox::warning(this,tr("连接检查"),tr("未能正常下载证书"),tr("确定"));
        //printMessage("证书未能下载");
    }
    */
    delete downloadCert;
#endif

}

/*************************************************************
 * Functin : set QT_GUI_AotuRun
 ************************************************************/
void MainWindow::AutoRun(bool bAuto)
{
    //自动启动程序
    QString str_vpn_exe_name = Con_Ini->value("setup/openvpn_exe_name").toString();
    QString szFileDir = (QCoreApplication::applicationDirPath() +"/"+str_vpn_exe_name);
    QSettings *reg = new QSettings("HKEY_LOCAL_MACHINE\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    //read firt
    qDebug()<<"reg before"<<reg->value("MeyAutoRun").toString();
    if(bAuto)
    {
        reg->setValue("MeyAutoRun",szFileDir.replace("/","\\"));
    }
    else
    {
        reg->setValue("MeyAutoRun"," ");

    }
    qDebug()<<"reg after"<<reg->value("MeyAutoRun").toString();

    delete reg;
}

/*************************************************************
 * Functin : show Conf
 ************************************************************/
void MainWindow::ShowConf()
{
    ui->label_19->setText(ui->ProtoBox->currentText());
    ui->label_20->setText(ui->comboBox_2->currentText());
    ui->label_21->setText(ui->lineEdit_server_ip->text());
    ui->label_22->setText(ui->lineEdit_server_port->text());
    ui->label_23->setText(ui->lineEdit_strategy_port->text());
}

/********************************************************
 * Function: ProxySetting
 * ******************************************************/
void MainWindow::ProxySetting()
{
    QString Proxy_Ip = ui->proxy_ip->text();
    QString Proxy_Port = ui->proxy_port->text();
    QString Proxy_Auto = ui->check_proxy->checkState()==2?"on":"off";

    QString ProxyServer = Proxy_Ip+":"+Proxy_Port;
    Con_Ini->setValue("proxy_setup/proxy_server_ip",Proxy_Ip);
    Con_Ini->setValue("proxy_setup/proxy_server_port",Proxy_Port);
    Con_Ini->setValue("proxy_setup/proxy_auto_setting",Proxy_Auto);

    //修改注册表
    //qDebug()<<"read before modified :"<<reg->value("m_proxy_server").toString();
    //qDebug()<<"Auto :"<<reg->value("m_proxy_auto_setting").toString();

    QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",QSettings::NativeFormat);

    reg->setValue("m_proxy_server",ProxyServer);
    reg->setValue("m_proxy_auto_setting",Proxy_Auto);
    /*
    if(Proxy_Auto=="on")
    {
        reg->setValue("ProxyServer",ProxyServer);
        reg->setValue("ProxyEnable",1);
    }else
    {
        reg->setValue("ProxyEnable",0);
    }
    */
    //'ProxyEnable', 0, _winreg.REG_DWORD, 1)
     //   _winreg.SetValueEx(ie_setting_reg, 'ProxyServer', 0, _winreg.REG_SZ, proxy_url)
    delete reg;
}

bool MainWindow::per_connect()
{
    printMessage("检查客户端版本...");
    printMessage("当前版本："+vpn_params->current_version);

    QString chk_version_url = vpn_params->check_version_url();
    qDebug() << "URL:" << chk_version_url;

    Check_Version ckv;
    if(!ckv.check(chk_version_url))
    {
        printMessage("检查版本失败，请检查配置:"+ckv.err_string);
        qDebug()<<"检查版本失败:" << ckv.err_string;
        return false;
    }

#if 0
    Http* http = new Http();

    QByteArray& array1= http->GetDownLoadUrl(chk_version_url);
    if(array1.isEmpty())
    {
        QMessageBox::warning(this,tr("检查版本"),tr("检查客户端版本失败"),tr("确定"));
        printMessage("检查版本失败，请检查配置");
        return false;
    }
#endif
    else
    {
        /*
        qDebug() << ckv.get_name();
        qDebug() << ckv.get_version();
        qDebug() << ckv.get_length();
        qDebug() << ckv.get_flag();
        */

        if(ckv.get_flag())
        {
            QMessageBox::warning(this,tr("检查版本"),tr("发现新版本"),tr("确定"));
            QString filename = myHelper::GetSaveFile("保存升级包",ckv.get_name());
            qDebug() << "保存升级文件到：" << filename;
            if(filename.isEmpty())
            {
                printMessage("未保存升级文件");
                return false;
            }

            Http updata;
            QString updata_url = vpn_params->updata_url();
            qDebug() << "Updata URL:" << updata_url;

            if(!updata.DownloadFile(filename,updata_url,ckv.get_length()))
            {
                printMessage("下载跟新包失败");
                return false;
            }
            else
            {
                printMessage("下载完成，请退出客户端，重新安装新版本");
                //这里要退出客户端然后启动自动运行安装程序

                QMessageBox::information(this,tr("下载成功"),"您的软件版本可以升级到"+ckv.get_version(),"关闭旧版本客户端");
                //启动安装程序
                //QProcess::startDetached(filename);
                //QProcess::execute(filename);
                //QByteArray test_ar = test.toLocal8Bit();
                //LPCSTR lp2 = _T(test_ar.constData());
                //ShellExecuteA(0,"open","explorer.exe",(filename.toLocal8Bit().constData()),NULL,SW_SHOWNORMAL);
                qApp->quit();

                return false;
            }
        }

        printMessage("已经是最新版本");

    }


    //return false;


    //printMessage("获取用户信息...");
    //[1] connect
//自测
#if 0
    QByteArray context = myHelper::ReadFile("request.xml");
    if(context.isEmpty())
    {
        qDebug()<<"获取请求正文失败"<<endl;
        return false;
    }
    QByteArray req = down->BuildReq(context);

   qDebug()<<"构造正文大小 : "<<context.size()<<endl;
   qDebug()<<"请求大小 : "<<req.size()<<endl;

#endif
    //测试只能自发自收
#if 1
    //[1] 选择读取证书
    /***********************************************/
    char *c_data = NULL;
    QString prop;
    if(vpn_params->remember_thumb)
    {
        prop = vpn_params->getStatus("thumb");
        qDebug() << "使用记住的证书 ：" << prop;
        if(prop.isEmpty())
        {
            prop = myHelper::RunCmd("Select_Cert.exe");
            Con_Ini->setValue("common/thumb",prop);
        }
    }
    else
    {
        prop =myHelper::RunCmd("Select_Cert.exe");
    }
    /***********************************************/
    qDebug()<<"选择证书："<<prop;
    //[2] 读取der编码的证书文件
   int  c_len = use_cryptoAPI_cert(&c_data,prop.toStdString().c_str());
   if(c_len <= 0)
   {
       qDebug()<< "获取证书失败";
       printMessage("获取用户信息失败");
       //down->err_string = "获取用户信息失败";
       return false;
   }
   if(c_data == NULL)
   {
       qDebug()<< "获取证书数据失败";
       printMessage("读取用户信息失败");
       down->err_string = "获取用户信息数据失败";
       return false;
   }
   qDebug()<<"获取证书数据成功";

   //[3] 证书数据base64编码
    QByteArray cert_base64 = QByteArray(c_data,c_len).toBase64();

    /*
        osType=x86
        certificate=
        ca_file_md5=
        cert_file_md5=
        key_file_md5=
    */
    //base64中有加号，要处理一下

    QString verify_url = vpn_params->verify_url_perfix();
    //cert_base64 = "\"" + cert_base64 + "\"";
    QTextCodec * codec = QTextCodec::codecForName("UTF-8");
    QByteArray byteArray = codec->fromUnicode(cert_base64);
    QByteArray byteArrayPercentEncoded = byteArray.toPercentEncoding();
    cert_base64 = byteArrayPercentEncoded;

    QString cert_item = vpn_params->add_verify_item("certificate",cert_base64);

    verify_url = verify_url + cert_item;
    verify_url += vpn_params->add_verify_item("cert_file_md5",vpn_params->getStatus("crt_md5"));
    verify_url += vpn_params->add_verify_item("ca_file_md5",vpn_params->getStatus("ca_md5"));
    verify_url += vpn_params->add_verify_item("key_file_md5",vpn_params->getStatus("key_md5"));

    //qDebug() << verify_url;



//不再使用tcp连接服务器
#ifdef tcpconnect
    QByteArray context = down->BuildContext_2(cert_base64,cert_base64.size());
    //qDebug()<<"base64 Cert:"<<cert_base64<<endl;
    free(c_data);
    //[4] 添加faceMan的协议头
   QByteArray req = down->BuildReq(context);
   qDebug()<<"证书文件大小 : "<<c_len;
   qDebug()<<"构造正文大小 : "<<context.size();
   qDebug()<<"请求大小     ："<<req.size();
#endif

#endif





   //printMessage("连接服务器...");
#if 0
    if(down->TryConnect(ui->lineEdit_server_ip->text(),ui->lineEdit_strategy_port->text().toInt()))
    {
        qDebug()<<"连接服务器成功 " << ui->lineEdit_server_ip->text() << ":" << ui->lineEdit_strategy_port->text();
    }
    else
    {
        qDebug()<<"连接服务器失败"<<endl;
        down->err_string = "连接服务器失败";
        return false;
    }

#endif



 //  printMessage("已连接到服务器");

#if 0
   //[5] 发送求情等待服务器相应
   //这里超时定为10s
   down->GetResponse(req);
   if(down->err_string.isEmpty())
   {
       qDebug()<<"获取服务器返回信息成功";
       //test start
        if(vpn_params->debug)
        {
       if(myHelper::SaveFile("ack_test.xml",down->_rt,0,down->_rt.size()))
           qDebug()<<"保存响应信息成功 ack_debug.xml"<< endl;
        }
       //test end
   }
   else
   {
       qDebug()<<"获取服务器响应失败:"<<down->err_string;
       return false;
   }
   //[6] 解析xml格式返回信息
   down->xml->load_xml(down->_rt);
   if(down->CheckResponse("Result","OK"))
   {
       qDebug()<<"身份认证成功！"<<endl;
       //返回是成功
   }
   else
   {
       qDebug()<<"身份认证失败！"<<endl;
       //获取错误字段
       down->err_string="身份认证失败，服务器拒绝了您的请求 "+down->xml->GetNode("ResultMsg");
       return false;
   }
   //保存处理xml文件中的各种项目 ca，客户端证书，配置文件等等
#endif

#if 1


    Verify_User vfy;
    if(!vfy.check(verify_url))
    {
        printMessage("获取服务器状态失败："+vfy.err_string);
        qDebug()<<"获取服务器状态失败:" << vfy.err_string;
        return false;
    }
    if(!vfy.get_bool("success"))
    {
        printMessage("身份认证失败："+vfy.get_string("msg"));
        qDebug()<<"身份认证失败:" << vfy.err_string;
        return false;
    }
    printMessage("身份认证成功");
    qDebug()<<"身份认证成功";

    if(vfy.get_bool("compare"))
    {
        printMessage("匹配配置数据成功");

        return true;
    }
    {
        printMessage("同步服务器配置...");
        QString config_md5 = vfy.get_string("config_md5");
        Con_Ini->setValue("others/config_md5",config_md5);
    /*
        QString key_md5 = vfy.get_string("key_md5");
        QString crt_md5 = vfy.get_string("crt_md5");
        QString ca_md5 = vfy.get_string("ca_md5");

        qDebug() << "n_key_md5:" << key_md5;
        qDebug() << "o_key_md5:" << vpn_params->getStatus("key_md5");

        qDebug() << "n_crt_md5:" << crt_md5;
        qDebug() << "o_crt_md5:" << vpn_params->getStatus("crt_md5");

        qDebug() << "n_ca_md5:" << ca_md5;
        qDebug() << "o_ca_md5:" << vpn_params->getStatus("ca_md5");

        qDebug() << "n_config_md5:" << config_md5;
        qDebug() << "o_config_md5:" << vpn_params->getStatus("config_md5");

    */


    }

    if(!vfy.sava_json("key",vpn_params->getStatus("key")))
    {
        qDebug() << "save key fail";
        printMessage("同步策略失败");
        return  false;
    }
    if(!vfy.sava_json("crt",vpn_params->getStatus("crt")))
    {
        qDebug() << "save crt fail";
        printMessage("同步策略失败");
        return  false;
    }
    if(!vfy.sava_json("ca",vpn_params->getStatus("ca")))
    {
        qDebug() << "save ca fail";
        printMessage("同步策略失败");
        return  false;
    }
    if(!vfy.sava_json("config",vpn_params->getStatus("config")))
    {
        qDebug() << "save config fail";
        printMessage("同步策略失败");
        return  false;
    }
#endif

    printMessage("身份认证成功");
    return true;
}




/*******************************************
 * Function : replace ip to download url
 *****************************************/
QString MainWindow::__replace_server_IP(const QString &strSourceURL)
{
    int nPos = 0;
    QString tmps = strSourceURL;
    nPos = tmps.indexOf("//");
    tmps = tmps.mid(nPos+2);
    nPos = tmps.indexOf(("/"));
    tmps = tmps.mid(nPos+1);

    return "http://" + ui->lineEdit_server_ip->text() +"/" +tmps;

}

QString MainWindow::__replace_download_server_IP(const QString &strSourceURL)
{
    int nPos = 0;
    QString tmps = strSourceURL;
    nPos = tmps.indexOf("//");
    tmps = tmps.mid(nPos+2);
    nPos = tmps.indexOf(("/"));
    tmps = tmps.mid(nPos+1);

    return "http://" + ui->lineEdit_server_ip->text()+":"+ui->lineEdit_strategy_port->text() +"/" +tmps;

}

/*************************************************************
 * Function: PrintMessage
 * ***********************************************************/
void MainWindow::printMessage(QString msg)
{
    if(msg.isEmpty())
        return;
   QDateTime current_date_time ;
   QString current_date ;
   current_date_time = QDateTime::currentDateTime();
   //current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss ddd");
   current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss :");
   current_date.append(msg);
   ui->textBrowser->append(current_date);
}

//翻译
QString MainWindow::TransforChinese(QString msg)
{
    return vpn_params->Translate_line(msg);
}




/******************************************************
 *  Function : Read from VPN process
 * ****************************************************/
void MainWindow::start_process()
{
    printMessage("SSLVPN启动成功,准备建立安全隧道");
    return;
}

void MainWindow::start_read_output()
{
    //char data[5]={'n','h','a','\r','\n'};
    QByteArray ba = vpnprocess->readAllStandardOutput();
    //printMessage(tr(ba));
    //vpnprocess->write(data,5);
    printMessage(TransforChinese(tr(ba)));
    WatchOpenVPNProcess();
    if(vpn_params->debug)
        printMessage(ba);
}

void MainWindow::start_read_err_output()
{
    //QByteArray ba = vpnprocess->readAllStandardError();
    QString ba = vpnprocess->readAllStandardError();
    //printMessage(trUtf8(ba));
   // printMessage(ba);
    //CheckMsg(ba);
    printMessage(TransforChinese(ba));
    WatchOpenVPNProcess();
    if(vpn_params->debug)
        printMessage(ba);
}

void MainWindow::finish_process(int exitCode, QProcess::ExitStatus exitStatus)
{
    vpn_params->ChangeStatus(VPN_DISCONNECT);
    printMessage("连接中断");
    WatchOpenVPNProcess();
}



/********************************************************
 * function : check for server
 * ******************************************************/
void MainWindow::newLocalSocketConnection()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
       if (!socket)
           return;
       socket->waitForReadyRead(1000);
       QTextStream stream(socket);
       //QMessageBox::warning(NULL,("安全网关"),("已经有客户端在运行"),("确定"));
       delete socket;
}

/********************************************************
 * function : check vpn message
 * ******************************************************/
void MainWindow::CheckMsg(QString msg)
{
    //非常重要
    //处理界界面变化
    //可能弹出密码输入框输入密码 ,可能需要转ascii
    //  "Enter PEM pass phrase:" ,22
    // "Enter Private Key Password:", 27
    // Enter Auth Username:", 20
    // "Enter Auth Password:", 20)
    //
    //可能需要维护一个结构保存连接信息
    int i;
    //连接中
   // printMessage(msg);
    if(msg.startsWith("Andy:TO_SERVER"))
    {
            ui->label_stat->setText(trUtf8("连接中..."));
            printMessage("开始建立连接...");
            ui->label_time->setText(trUtf8("00:00:00"));
            ui->label_addr->setText(trUtf8("-.-.-.-"));
        //设置返回按钮为不可用
            ui->pushButton_back->setDisabled(true);
        //设置断开按钮可用
            ui->pushButton_disconnection->setDisabled(false);
        //设置托盘图标为连接中
            QIcon icon(":/connectting.png");
            trayIcon->setIcon(icon);
            setWindowIcon((icon));
    }
    //已连接
    else if(msg.startsWith("Andy:CONNECTED"))
    {
        i = msg.indexOf("IP");
        /*time start*/
        QDateTime current_date_time ;
        QString current_date ;
        current_date_time = QDateTime::currentDateTime();
        current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss ddd");
        ui->label_time->setText((current_date));

        /*time end*/
        ui->label_stat->setText(trUtf8("已连接"));
        ui->label_addr->setText((msg.mid(i+3)));
        ui->toolBox->setCurrentIndex(0);
        //设置托盘图标为绿色
            QIcon icon(":/connectted.png");
            trayIcon->setIcon(icon);
            setWindowIcon((icon));
        //设置返回按钮为不可用
            ui->pushButton_back->setDisabled(true);
        //设置断开按钮为可用
            ui->pushButton_disconnection->setDisabled(false);
         printMessage("连接成功");

    }
    //已停止
    else if(msg.startsWith("OPENVPN_STOP"))
    {
        ui->label_stat->setText(trUtf8("未连接"));
        ui->label_time->setText(trUtf8("00:00:00"));
        ui->label_addr->setText(trUtf8("-.-.-.-"));
        //设置托盘图标为灰色
            QIcon icon(":/disconnection.png");
            trayIcon->setIcon(icon);
            setWindowIcon((icon));
        //恢复返回按钮可用
            ui->pushButton_back->setDisabled(false);
        //设置断开俺就为不可用
            ui->pushButton_disconnection->setDisabled(true);

    }
    //重连中
    else if(msg.startsWith("RECONNECTING"))
    {
        ui->label_stat->setText(trUtf8("重连中..."));
        ui->label_time->setText(trUtf8("00:00:00"));
        ui->label_addr->setText(trUtf8("-.-.-.-"));
        //设置返回按钮为不可用
            ui->pushButton_back->setDisabled(true);
        //设置断开按钮可用
            ui->pushButton_disconnection->setDisabled(false);
        //设置托盘图标为连接中
            QIcon icon(":/connectting.png");
            trayIcon->setIcon(icon);
            setWindowIcon((icon));
    }
    /**其他更多的信息****************************/
    //可用网段及掩码
    else if(msg.startsWith("Andy:IP_MASK"))
    {
        i = msg.indexOf("MASK");
        ui->label_mask->setText(msg.mid(i+5));
        //printMessage(msg.mid(i+5));
    }
    // Error Information
    else if(msg.trimmed().startsWith("AERROR"))
    {
        //QMessageBox::information(this,tr("LOADCERT"),QString::number(msg.indexOf("LOAD_CERAT")),tr("确定"));
        //QMessageBox::information(this,tr("ERR_TO_SERVER"),QString::number(msg.indexOf("ERR_TO_SER")),tr("确定"));

        // printMessage("hello:" + msg.indexOf("LOAD_CERT"));
        if(msg.indexOf("LOAD_CERT") > 0)
        {
            printMessage("未能正确载入证书，请检查");
        }
        else if(msg.indexOf("CON_ERR") > 0)
        {
            printMessage("连接过程发生错误，准备退出！");
        }
        else if(msg.indexOf("ERR_TO_SER") > 0)
        {
            printMessage("连接到服务器失败，5s后再次尝试！");
        }
        else if(msg.indexOf("NO_DEV") > 0)
        {
            printMessage("未安装驱动程序，请检查!");
        }

        {
            ui->label_stat->setText(trUtf8("未连接"));
            ui->label_time->setText(trUtf8("00:00:00"));
            ui->label_addr->setText(trUtf8("-.-.-.-"));
            //设置托盘图标为灰色
            QIcon icon(":/disconnection.png");
            trayIcon->setIcon(icon);
            setWindowIcon((icon));
            //恢复返回按钮可用
            ui->pushButton_back->setDisabled(false);
            //设置断开俺就为不可用
            ui->pushButton_disconnection->setDisabled(true);
        }
    }
    //一些不是很可靠的提示信息
    else if(msg.trimmed().startsWith("Andy"))
    {

       if(msg.indexOf("FORB") > 0)
       {
           printMessage("用户已被禁止访问");
       }
       else if(msg.indexOf("NOT_FOUND") > 0)
       {
           printMessage("服务器未识别此用户，可能已被吊销！");
       }
       ui->label_stat->setText(trUtf8("未连接"));
       ui->label_time->setText(trUtf8("00:00:00"));
       ui->label_addr->setText(trUtf8("-.-.-.-"));
       //设置托盘图标为灰色
       QIcon icon(":/disconnection.png");
       trayIcon->setIcon(icon);
       setWindowIcon((icon));
       //恢复返回按钮可用
       ui->pushButton_back->setDisabled(false);
       //设置断开俺就为不可用
       ui->pushButton_disconnection->setDisabled(true);

    }
    else
        printMessage(msg);



#if 0
    if(msg.startsWith("Andy"))
    {
        printMessage("进入Andy");
        if(msg.indexOf("CONNECTED"))
       {
           i = msg.indexOf("-");
           /*time start*/
           QDateTime current_date_time ;
           QString current_date ;
           current_date_time = QDateTime::currentDateTime();
           current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss ddd");
            ui->label_time->setText((current_date));

           /*time end*/
            ui->label_stat->setText(trUtf8("已连接"));
            ui->label_addr->setText((msg.mid(i+1)));
            ui->toolBox->setCurrentIndex(0);

       }
        if(msg.indexOf("CONNECTING"))
       {
            ui->label_stat->setText(trUtf8("连接中..."));
            printMessage(":开始建立连接...");
            ui->label_time->setText(trUtf8("00:00:00"));
            ui->label_addr->setText(trUtf8("-.-.-.-"));
       }
       else if(msg.indexOf("DISCONNECT"))
       {
            ui->label_stat->setText(trUtf8("未连接"));
            ui->label_time->setText(trUtf8("00:00:00"));
            ui->label_addr->setText(trUtf8("-.-.-.-"));
            ui->toolBox->setCurrentIndex(1);
       }
       else if(msg.indexOf("RECONNECT"))
       {
            ui->label_stat->setText(trUtf8("正在重连"));
            ui->label_time->setText(trUtf8("00:00:00"));
            ui->label_addr->setText(trUtf8("-.-.-.-"));

       }
       else if(msg.indexOf("VITTUALIP"))
       {

       }
       else if(msg.indexOf("IP_MASK"))
       {
           i = msg.indexOf("-");
           printMessage("可用网段及掩码");
           printMessage(msg.mid(i+1));

       }
       //more information
       else if(msg.indexOf("TO_SERVER"))
       {
           printMessage("准备连接服务器...");
       }
       else if(msg.indexOf("TO_INIT"))
       {
           printMessage("初始化完成");
       }
       else if(msg.indexOf("FORB"))
       {
           printMessage("用户已被禁止访问");
       }
       else if(msg.indexOf("NOT_FOUND"))
       {
           printMessage("服务器未识别此用户，可能已被吊销！");
       }
    }
    else if(msg.startsWith("AERROR"))
    {
        if(msg.indexOf("LOAD_CERT"))
            printMessage("未能正确载入证书，请检查");
        else if(msg.indexOf("CON_ERR"))
            printMessage("连接过程发生错误，准备退出！");
        else if(msg.indexOf("ERR_TO_SER"))
            printMessage("连接到服务器失败，5s后再次尝试！");
        else if(msg.indexOf("NO_DEV"))
            printMessage("未安装驱动程序，请检查!");



    }
    else
        printMessage(msg);
#endif

    //add by andy 2015-12-02 处理原始的输出信息


}

bool MainWindow::IsMsg(QString msg, QString key)
{
    if(msg.contains(key))
        return true;
    return false;
}








/************************************************
 * Function : On_Cliecked Login
 ************************************************/
void MainWindow::on_pushButton_Login_clicked()
{
    //test query
    /*
    QUrl url1("http://192.168.1.214:80/verify_action?cert=I+T=DKJEDF+dDed&cert_md5=8877665544");

    QUrl url2(url1);
    QUrlQuery q;
    q.addQueryItem("success","123+3456+hahah");

    url2.setQuery(q.query(QUrl::FullyEncoded).toUtf8());

    qDebug() <<"Query:"<<url2.toString();

    return ;
    */

    //切换到登陆状态显示界面
    ui->stackedWidget->setCurrentIndex(1);

    // [0] 准备，清理之前残留的进程或者其他资源
    // [1] 打开证书选择对话框选择证书，并将证书数据base64编码
    // [2] 将证书数据封装成带头的xml数据
    // [3] 发送数据等待验证结果，（跳转到登陆界面，输出相关日志）
    // [4] 验证失败，停留在登陆日志界面，手动返回
    // [5] 验证成功，处理接收到的数据，该保存成文件的，该填充参数的，
    // [6] 文件和数据都准备好，准备启动VPN
    // [7] 处理VPN的输出信息

    //初始化参数
    vpn_params->Status_map.clear();
    //从界面上获取信息
    vpn_params->setStatus("server_ip",ui->lineEdit_server_ip->text());
    vpn_params->setStatus("server_port",ui->lineEdit_server_port->text());
    vpn_params->setStatus("verify_port",ui->lineEdit_strategy_port->text());
    vpn_params->setStatus("proto",ui->ProtoBox->currentText());
    //初始化虚拟ip
    vpn_params->setStatus("IP","");
    vpn_params->setStatus("IP_Mask","");

    //记住证书序列号？
    if(vpn_params->remember_thumb)
        vpn_params->setStatus("thumb",vpn_params->thumb);

    QString version = myHelper::RunCmd(vpn_params->exe + " --version");
    version =  version.split(" ")[1];
    vpn_params->exe_version = version;
    qDebug() << "Version " << version;

    QString current_path = vpn_params->work_path;

    vpn_params->setStatus("key",current_path+"/client.key");
    vpn_params->setStatus("crt",current_path+"/client.crt");
    vpn_params->setStatus("ca",current_path+"/ca.crt");
    vpn_params->setStatus("config",current_path+"/client.ovpn");

    vpn_params->setStatus("key_md5",myHelper::GetFileMD5(current_path+"/client.key"));
    vpn_params->setStatus("crt_md5",myHelper::GetFileMD5(current_path+"/client.crt"));
    vpn_params->setStatus("ca_md5",myHelper::GetFileMD5(current_path+"/ca.crt"));
    vpn_params->setStatus("config_md5",vpn_params->config_md5);


    /**********************************************************/
    // 上传验证签名证书，下载配置信息及策略
    down->err_string.clear();
    //显示连接过程中的信息

    if(!per_connect())
    {
        //qDebug()<<"配置安全参数失败:" << down->err_string;
        //printMessage("配置安全参数失败："+down->err_string);
        return;
    }

    /************************************************************/

    // [0]
    ShowConf();
    if(ui->check_autoRun->checkState() == 2)
    {
        Con_Ini->setValue("setup/openvpn_exe_auto","on");
        AutoRun(true);
    }
    else
    {
        AutoRun(false);
        Con_Ini->setValue("setup/openvpn_exe_auto","off");
    }

    StopVpn();

    printMessage("准备启动SSLVPN");

    // add new vpn process
    vpnprocess = new QProcess(this);

    //启动参数
    //QDir::setCurrent(QApplication::applicationDirPath().append("/test"));
    qDebug() << "运行目录:" <<QDir::currentPath();

    //修改配置文件
    QString tmp_ovpn = vpn_params->getStatus("config");
    //ReConfigOvpn(tmp_ovpn.mid(1,tmp_ovpn.length()-2));
    ReConfigOvpn(tmp_ovpn);

    //QString exe = "openvpn --config client_110.ovpn --pkcs12 client_with_ca.pfx";

    vpn_params->cmd_line = vpn_params->exe +
            " --config " + vpn_params->getStatus("config") +
            " --cert " + vpn_params->getStatus("crt") +
            " --key " + vpn_params->getStatus("key") +
            " --ca " + vpn_params->getStatus("ca");

    //if httpproxy
    QString httpproxy = "";
    if(!ui->proxy_ip->text().trimmed().isEmpty() && !ui->proxy_port->text().trimmed().isEmpty())
    {
        /*
        httpproxy = " --http-proxy " +ui->proxy_ip->text().trimmed() + " " +ui->proxy_port->text().trimmed();
        vpn_params->cmd_line = vpn_params->cmd_line + httpproxy;
        */
        QString Proxy_Ip = ui->proxy_ip->text();
        QString Proxy_Port = ui->proxy_port->text();
        QString Proxy_Auto = ui->check_proxy->checkState()==2?"on":"off";

        QString ProxyServer = Proxy_Ip+":"+Proxy_Port;
        QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",QSettings::NativeFormat);
        if(Proxy_Auto=="on")
        {
            reg->setValue("ProxyServer",ProxyServer);
            reg->setValue("ProxyEnable",1);
        }else
        {
            reg->setValue("ProxyEnable",0);
        }
        delete reg;
    }

    QString exe = vpn_params->cmd_line;
    qDebug()<<"CMD line: ["<< exe <<  "]";

    //printMessage(exe);
    connect(vpnprocess,SIGNAL(started()),this,SLOT(start_process()));
    connect(vpnprocess,SIGNAL(readyReadStandardOutput()),this,SLOT(start_read_output()));
    connect(vpnprocess,SIGNAL(readyReadStandardError()),this,SLOT(start_read_err_output()));
    connect(vpnprocess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finish_process(int, QProcess::ExitStatus)));

    vpn_params->ChangeStatus(VPN_CONNECTING);
    WatchOpenVPNProcess();

    vpnprocess->start(exe);

    if(false == vpnprocess->waitForStarted())
    {
        printMessage("SSLVPN 启动失败，可能是主程序损坏");
        //return;
    }
}


/************************************************
 * Function : On_Cliecked Test
 ************************************************/
void MainWindow::on_pushButton_Test_clicked()
{
    ui->pushButton_Test->setDisabled(true);
    QTcpSocket *client;
    client=new QTcpSocket(this);


    if(ui->ProtoBox->currentText() == "tcp")
    {
        client->connectToHost(QHostAddress(ui->lineEdit_server_ip->text()),ui->lineEdit_server_port->text().toInt());
        if(client->waitForConnected(2000))
        {
            //stat = 0;
            //QMessageBox::information(this,tr("连接检查"),tr("连接成功"));
        }else
        {
            QMessageBox::warning(this,tr("连接检查"),tr("服务器连接失败，请检查网络"),tr("确定"));
            client->close();
            delete client;
            ui->pushButton_Test->setDisabled(false);
            return;
        }
        client->close();
    }

    client->connectToHost(QHostAddress(ui->lineEdit_server_ip->text()),ui->lineEdit_strategy_port->text().toInt());
    if(client->waitForConnected(2000))
    {
        QMessageBox::information(this,tr("连接检查"),tr("服务器连接正常"),tr("确定"));
    }else
    {
        QMessageBox::warning(this,tr("连接检查"),tr("服务器连接失败，请检查网络"),tr("确定"));
    }
    client->close();
    delete client;
   ui->pushButton_Test->setDisabled(false);




#if 0
    QString IP = ui->lineEdit_server_ip->text();
    QString Port1 = ui->lineEdit_server_port->text();


    //QString strArg = "check_alive "+ IP +" "+ Port;
    QString strArg = "MyGet -t "+ IP +" "+ Port;
   int exitCode = QProcess::execute(strArg);
   qDebug()<<"exitCode :"<< exitCode;
   if(0 == exitCode)
   {
       QMessageBox::information(this,tr("连接检查"),tr("服务器连接正常"),tr("确定"));
   }else
   {
        QMessageBox::warning(this,tr("连接检查"),tr("服务器连接失败，请检查网络"),tr("确定"));

   }
#endif
}


/************************************************
 * Function : On_Cliecked Save Config
 ************************************************/
void MainWindow::on_pushButton_ConfigApply_clicked()
{
    int ret = 0;
    ui->pushButton_ConfigApply->setDisabled(true);

    // 空校验
    if((ui->lineEdit_server_ip->text().isEmpty()) || ui->lineEdit_server_port->text().isEmpty() || ui->lineEdit_strategy_port->text().isEmpty())
    {
        QMessageBox::warning(this,tr("保存配置"),tr("不能有空参数"),tr("确定"));
        return;
    }
    //要先读取ui上的地址获取下载地址
    UI_2_Conf();
// 不再通过http请求下载
#if 0
    ret = DownloadCert();
    //wait for cert download done
    //Sleep(2000);
    //重新读取配置文件并修改配置文件
    m_pConfigFile->ReLoadCfgFile();
   // DownloadCert();
    if(ret == 0 )
    {
        QMessageBox::information(this,tr("保存配置"),tr("配置保存成功"),tr("确定"));
    }
#endif
    ui->pushButton_ConfigApply->setDisabled(false);
    QMessageBox::information(this,tr("保存配置"),tr("配置保存成功"),tr("确定"));

}

/************************************************
 * Function : On_Cliecked Save Proxy
 ************************************************/
void MainWindow::on_pushButton_ProxyApply_clicked()
{
    // 空校验
    /*
    //浏览器代理可以不设置:
    if((ui->proxy_ip->text().isEmpty()) || ui->proxy_port->text().isEmpty())
    {
        QMessageBox::warning(this,tr("保存配置"),tr("不能有空参数"),tr("确定"));
        return;
    }
    */
    ProxySetting();
    QMessageBox::information(this,tr("浏览器代理配置"),tr("配置保存成功"),tr("确定"));
}

/************************************************
 * Function : On_Cliecked cancelSetting
 ************************************************/
void MainWindow::on_pushButton_ProxyCancel_clicked()
{
    Conf_2_UI();
}

void MainWindow::on_pushButton_ConfigCancel_clicked()
{
    Conf_2_UI();
}


/************************************************
 * Function : On_Cliecked disconnecting
 ************************************************/
void MainWindow::on_pushButton_disconnection_clicked()
{
    //干掉openvpn
    QProcess *poc = new QProcess;
    //poc->start("kill_openvpn.bat");
    poc->execute("kill_openvpn.bat");

    ui->label_stat->setText(trUtf8("未连接"));
    ui->label_time->setText(trUtf8("00:00:00"));
    ui->label_addr->setText(trUtf8("-.-.-.-"));
    ui->label_mask->setText(trUtf8(""));
    //设置托盘图标为灰色
    QIcon icon(":/disconnection.png");
    trayIcon->setIcon(icon);
    setWindowIcon((icon));
    //恢复返回按钮可用
    ui->pushButton_back->setDisabled(false);
    //设置断开俺就为不可用
    ui->pushButton_disconnection->setDisabled(true);
    //设置浏览器代理断开
    QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",QSettings::NativeFormat);

    //reg->setValue("ProxyEnable","00000000");
    reg->setValue("ProxyEnable",false);
    //reg->setValue("m_proxy_auto_setting","off");

    delete poc;

}


/************************************************
 * Function : On_Cliecked back
 ************************************************/
void MainWindow::on_pushButton_back_clicked()
{
    QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",QSettings::NativeFormat);
    reg->setValue("ProxyEnable",false);

    QProcess *poc = new QProcess;
    //poc->start("kill_openvpn.bat");
    poc->execute("kill_openvpn.bat");
    delete poc;
    ui->stackedWidget->setCurrentIndex(0);
    ui->textBrowser->clear();
}

void MainWindow::on_check_autoRun_clicked()
{

    if(ui->check_autoRun->checkState() == 2)
    {
        Con_Ini->setValue("setup/openvpn_exe_auto","on");
        AutoRun(true);
    }
    else
    {
        AutoRun(false);
        Con_Ini->setValue("setup/openvpn_exe_auto","off");
    }
}

// Delect m_key.txt to rechose cert
/*
void MainWindow::on_resetButton_clicked()
{
    QFile f;
    f.remove("m_key.txt");
}
*/


//根据状态转换处理不同的状态
void MainWindow::WatchOpenVPNProcess()
{
    //处理显示

    //处理状态转移
    if(vpn_params->current_status == vpn_params->last_status)
    {
        //无状态变化，无需处理；
        return;
    }
    switch(vpn_params->current_status)
    {
    case VPN_CONNECTING:
    {
        ui->label_stat->setText(trUtf8("连接中..."));
        printMessage("开始建立连接...");
        ui->label_time->setText(trUtf8("00:00:00"));
        ui->label_addr->setText(trUtf8("-.-.-.-"));
        ui->label_mask->clear();
        //设置返回按钮为不可用
        ui->pushButton_back->setDisabled(true);
        //设置断开按钮可用
        ui->pushButton_disconnection->setDisabled(false);
        //设置托盘图标为连接中
        QIcon icon(":/connectting.png");
        trayIcon->setIcon(icon);
        setWindowIcon((icon));
        break;
    }
    case VPN_CONNECTED:
    {
        /*time start*/
        QDateTime current_date_time ;
        QString current_date ;
        current_date_time = QDateTime::currentDateTime();
        current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss ddd");
        ui->label_time->setText((current_date));

        /*time end*/
        ui->label_stat->setText(trUtf8("已连接"));
        ui->label_addr->setText((vpn_params->Status_map["IP"]));
        ui->label_mask->setText(vpn_params->Status_map["IP_Mask"]);
        ui->toolBox->setCurrentIndex(0);
        //设置托盘图标为绿色
        QIcon icon(":/connectted.png");
        trayIcon->setIcon(icon);
        setWindowIcon((icon));
        //设置返回按钮为不可用
        ui->pushButton_back->setDisabled(true);
        //设置断开按钮为可用
        ui->pushButton_disconnection->setDisabled(false);
        printMessage("连接成功");
    }
        break;
    case VPN_DISCONNECT:
    {
        ui->label_stat->setText(trUtf8("未连接"));
        ui->label_time->setText(trUtf8("00:00:00"));
        ui->label_addr->setText(trUtf8("-.-.-.-"));
        ui->label_mask->clear();
        //设置托盘图标为灰色
        QIcon icon(":/disconnection.png");
        trayIcon->setIcon(icon);
        setWindowIcon((icon));
        //恢复返回按钮可用
        ui->pushButton_back->setDisabled(false);
        //设置断开俺就为不可用
        ui->pushButton_disconnection->setDisabled(true);
        break;
    }
    case VPN_RECONNECTING:
    {
        ui->label_stat->setText(trUtf8("重连中..."));
        ui->label_time->setText(trUtf8("00:00:00"));
        ui->label_addr->setText(trUtf8("-.-.-.-"));
        ui->label_mask->clear();
        //设置返回按钮为不可用
        ui->pushButton_back->setDisabled(true);
        //设置断开按钮可用
        ui->pushButton_disconnection->setDisabled(false);
        //设置托盘图标为连接中
        QIcon icon(":/connectting.png");
        trayIcon->setIcon(icon);
        setWindowIcon((icon));
        break;
    }
    default:
        break;
    }

    //状态转换完成，保持状态不变化
    vpn_params->last_status = vpn_params->current_status;

}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if(msgType==WM_DEVICECHANGE)
    {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags ==0)
                {
                    //qDebug() << "USB_Arrived  type : The USBDisk ";
                }
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags == 0)
                {
                    //qDebug() << "USB_Removed";

                }
            }
            if(lpdb->dbch_devicetype = DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE pDevInf  = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;
                //qDebug()<< "移除设备(name)：" << pDevInf->dbcc_name;
                //qDebug()<< "移除设备(size)：" << pDevInf->dbcc_size;
                QString strname = QString::fromWCharArray(pDevInf->dbcc_name,pDevInf->dbcc_size);

                //蓝Key的标识 Vid_5448&Pid_0001
                if(strname.contains("Vid_5448") ||
                        strname.contains("VID_5448") ||
                        strname.contains("_5448"))
                {
                    qDebug() << "USB KEY 移除";
                    StopVpn();
                    printMessage("加密设备被移除");
                }

            }
            break;

        }
    }
    return false;
}

void MainWindow::ReConfigOvpn(QString ofile)
{
   //配置文件的部分选项可能需要修改
    //1.修改remote

    QFile file(ofile);
    QStringList  conf_list;
    QTextStream textStream;
    int once = 0;
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "没有找到配置文件:" << ofile;
    }
    else
    {
        //textStream(&file);
        textStream.setDevice(&file);
        while(!textStream.atEnd())
        {
            QString tmp_line = textStream.readLine();
            QString keyStr = tmp_line.split(" ")[0];
            if(keyStr == "remote" && (once == 0))
            {
                tmp_line = "remote " + ui->lineEdit_server_ip->text() + " " +
                        ui->lineEdit_server_port->text();
                once = 1;
            }
            if(keyStr == "proto" )
            {
                tmp_line = "proto " + ui->ProtoBox->currentText();
            }
            if(keyStr == "ca" || keyStr == "cert" || keyStr == "key" || keyStr == "pkcs12")
                continue;

            conf_list.append(tmp_line);
        }
    }
    file.close();

    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate| QIODevice::Text))
    {
        qDebug() << "修改时没有找到配置文件:" << ofile;
    }
    else
    {
        textStream.setDevice(&file);

        qDebug() << "修改配置文件" << endl;
        for(int i = 0;i < conf_list.count();i++)
        {
            textStream << conf_list.operator [](i) << "\r\n";
        }
    }
    file.close();

}

//关停vpn
void MainWindow::StopVpn()
{
    QProcess *poc = new QProcess;
    //poc->start("kill_openvpn.bat");
    poc->execute("kill_openvpn.bat");

    delete poc;

    //设置浏览器代理断开
    QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SoftWare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",QSettings::NativeFormat);

    //reg->setValue("ProxyEnable","00000000");
    reg->setValue("ProxyEnable",false);
    //reg->setValue("m_proxy_auto_setting","off");
    vpn_params->ChangeStatus(VPN_DISCONNECT);
    WatchOpenVPNProcess();
}

//记住证书
void MainWindow::on_checkBox_thumb_clicked()
{
    if(ui->checkBox_thumb->isChecked())
    {
        Con_Ini->setValue("common/remember_thumb","yes");
        vpn_params->remember_thumb = true;
    }
    else
    {
        Con_Ini->setValue("common/remember_thumb","no");
        vpn_params->remember_thumb = false;
        Con_Ini->setValue("common/thumb","");
    }
}
