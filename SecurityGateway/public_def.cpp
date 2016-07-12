#include "public_def.h"
#include <QUrl>
#include <QDebug>


VpnParams::VpnParams(QObject *parent)
{
    exe         = "openvpn.exe";
    exe_version = DEFAULT_VERSION;
    cmd_line    = exe;

    //remote.clear();
    //proto       = "tcp";

    conf_file       = "client.ovpn";
    conf_dir        = "./";

    log_file        = "sslvpn.log";

    current_status  = VPN_DISCONNECT;
    last_status     = VPN_DISCONNECT;

    auto_connect    = false;
    remember_thumb   = false;
    thumb.clear();

    failed_psw          = 0;
    failed_psw_attempts = 3;

    restart = false;
    debug   = false;

    Status_map.clear();
    //ovpn_map.clear();
}

VpnParams::~VpnParams()
{

}

void VpnParams::setStatus(QString key, QString Value)
{
    Status_map.insert(key,Value);
}

QString VpnParams::getStatus(QString key)
{
     if(Status_map.contains(key)){
         return Status_map[key];
     }
     else
         return NULL;
}


//将输出翻译成中文，并处理状态转换
QString VpnParams::Translate_line(QString msg)
{
    static int nCount = 0;
    QString rtString;
    rtString.clear();
    if(msg.contains("NOTE: debug verbosity (--verb 9) is enabled but this build lacks debug support"))
    {
        ChangeStatus(VPN_CONNECTING);
        rtString = "开始建立连接...";
    }
    else if(msg.contains("Cannot load certificate"))
    {
        ChangeStatus(VPN_DISCONNECT);
        rtString = "未能正确载入证书，请检查...";
        //remove(afile_path);
    }
    else if(msg.contains("Exiting due to fatal error"))
    {
        ChangeStatus(VPN_DISCONNECT);
        rtString= "连接过程发生错误，准备退出!";
        //remove(afile_path);
    }
    else if(msg.contains("will try again in 5 seconds: Connection refused"))
    {
        ChangeStatus(VPN_RECONNECTING);
        rtString= "连接到服务器失败，5s后再次尝试!";
        //remove(afile_path);
    }
    else if(msg.contains("There are no TAP-Win32 adapters on this system"))
    {
        ChangeStatus(VPN_DISCONNECT);
        rtString= "未安装驱动程序，请检查!";
        //remove(afile_path);
    }
    else if(msg.contains("as a OpenVPN static key file"))
    {
        ChangeStatus(VPN_CONNECTING);
        nCount = 0;
        rtString= "准备连接服务器...";
    }
    else if(msg.contains("Error opening configuration file"))
    {
        ChangeStatus(VPN_DISCONNECT);
        rtString= "未找到配置信息";
    }
    else if(msg.contains( "Initialization Sequence Completed"))
    {
        ChangeStatus(VPN_CONNECTED);
        rtString= "建立安全隧道成功";
    }
    /*
    //这里翻译不准确，不是被禁止
    else if(msg.contains( "'PUSH_REQUEST' (status=1)"))
    {
        nCount++;
        if(nCount >= 2)
        {
            nCount = 0;
            rtString= "用户已被禁止访问！";
            ChangeStatus(VPN_DISCONNECT);
            //remove(afile_path);
        }
    }
    */
    else if(msg.contains("SIGUSR1[soft,connection-reset] received"))
    {
        ChangeStatus(VPN_DISCONNECT);
        rtString= "服务器未识别此用户，可能已被吊销！";
        //remove(afile_path);
    }
    else if(msg.contains("Unrecognized option or missing parameter"))
    {
        ChangeStatus(VPN_DISCONNECT);
        rtString= "无效的参数或选项!";
    }
//这种写法需要runtimelib库的支持
#if 0
    else if(msg.contains("PUSH: Received control message: 'PUSH_REPLY,"))
    {
        /*
        Tue Nov 12 18:19:26 2013 PUSH: Received control message: 'PUSH_REPLY,route 192.168.4.0 255.255.255.0,route 192.168.6.0 255.255.255.0,dhcp-option DOMAIN domain.com,route 10.8.0.0 255.255.255.0,topology net30,ping 10,ping-restart 60,ifconfig 10.8.0.26 10.8.0.25'
        */
        int nPos = 0;
        char* pHead = msg.toStdString().c_str();
        char* pc = NULL;
        char* p_ = NULL;//逗号
        char* p_m = NULL;
        char tmplst[32];
        char iplst[256];
        memset(iplst, 0, 256);

        p_m = strstr(pHead, "dhcp-option DOMAIN");
        if(p_m == NULL)
            return;

        while(strstr(pHead, "route") != NULL)
        {
            memset(tmplst, 0, 32);
            pc = strstr(pHead, "route");
            if(pc > p_m)
                break;
            pc = pc + 5;//删除“route”

            p_ = strstr(pc, ",");
            if(p_ == NULL)
                break;
            memcpy(tmplst, pc, p_-pc);
            strcat(iplst, "\r\n");
            strcat(iplst, tmplst);
            pHead = p_ + 1;
        }


        rtString= "可用网段及掩码：" + QString(iplst);
    }
#endif
    else if(msg.contains("PUSH: Received control message: 'PUSH_REPLY,"))
    {
        /*
        Tue Nov 12 18:19:26 2013 PUSH: Received control message: 'PUSH_REPLY,route 192.168.4.0 255.255.255.0,route 192.168.6.0 255.255.255.0,dhcp-option DOMAIN domain.com,route 10.8.0.0 255.255.255.0,topology net30,ping 10,ping-restart 60,ifconfig 10.8.0.26 10.8.0.25'
        */

        /*
         DHCP IP/netmask of 10.8.0.14/255.255.255.252
        */

         /*
          route.exe ADD 10.8.0.1 MASK 255.255.255.255
          */

        QString ip_mask;
        ip_mask.clear();

        int pos = 0;
        int route = 0;
        int _p=  0 ; //逗号
        int end = msg.indexOf("dhcp-option DOMAIN");
        qDebug() << "msg:" << msg;

        if(end > 0)
        {
            do{
                route = msg.indexOf("route",pos);
                pos = route + 5;
                _p = msg.indexOf(",",pos);
                if(_p > end)
                    break;
                ip_mask = ip_mask +" "+ msg.mid(pos,_p-pos)+"\n";
                //qDebug() << "ip_mask:" << msg.mid(pos,_p-pos) << endl;
            }while(route > 0 );
            rtString = "可用网段及掩码："+ip_mask;
            Status_map["IP_Mask"] = ip_mask;

            //rtString = "可用网段及掩码："+ msg;
        }
       // qDebug() << "rtString" << rtString;
    }
    else if(msg.contains("Notified TAP-Win32 driver to set a DHCP IP"))
    {
        //获取虚拟ip
        int start = 0;
        start = msg.indexOf("Notified TAP-Win32");
        QString IP = msg.mid(start+54,15).simplified();
        int end = 0;
        if((end = IP.indexOf("/")) > 0 )
            IP = IP.mid(0,end);
        else if((end = IP.indexOf(" ")) > 0 )
            IP = IP.mid(0,end);


        Status_map["IP"] = IP;
        rtString = "获取虚拟地址："+IP;


    }
    else if(msg.contains("process restarting"))
    {
        ChangeStatus(VPN_RECONNECTING);
        rtString = "重新连接...";
    }else if(msg.contains("certificate is not yet valid") || msg.contains("certificate is not yet valid"))
    {
        ChangeStatus(VPN_DISCONNECT);
        rtString= "证书过期或证书未生效，请检查系统时间!";
    }

    return rtString;
}


void VpnParams::ChangeStatus(int status)
{
    /*
    if(current_status == status)
        return;
    if(current_status != status)
    {
       last_status = current_status;
       current_status = status;
    }
    */
    if(current_status != status)
    {
       //last_status = current_status;
       current_status = status;
    }
}

QString VpnParams::check_version_url()
{
    QString ret = "http://"+getStatus("server_ip")+":" +
            getStatus("verify_port") +
            check_version + "?"
            "os=" + os_type + "&" +
            "version="+current_version;
    return ret;
}

QString VpnParams::updata_url()
{
    QString ret = "http://"+getStatus("server_ip")+":" +
            getStatus("verify_port") +
            down_exe + "?"
            "os=" + os_type;
    return ret;
}

QString VpnParams::verify_url_perfix()
{
    QString ret = "http://"+getStatus("server_ip")+":" +
            getStatus("verify_port") +
            verify + "?"
                       "osType=" + os_type;
    return ret;
 }

QString VpnParams::add_verify_item(QString item, QString value)
{
    QString ret= "&" + item + "=" + value;
    return ret;
}

QUrl VpnParams::verify_url_perfix_encode()
{
    QString ret = "http://"+getStatus("server_ip")+":" +
            getStatus("verify_port") +
            verify + "?"
                       "osType=" + os_type;
    return QUrl(ret);

}

QUrl VpnParams::add_verify_item_encode(QUrl url,QString item, QString value)
{

}
