#ifndef PUBLIC_DEF_H
#define PUBLIC_DEF_H

#include <QObject>
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QMap>
#include <QTemporaryFile>
#include <QTemporaryDir>
//配置文件的key

#define KEY_PROTO	"proto"
#define KEY_DEV    "dev"
#define KEY_REMOTE "remote"
#define KEY_VERB   "verb"
#define KEY_CA     "ca"

//key字符串长度
#define KEY_PROTO_LEN  5
#define KEY_DEV_LEN    3
#define KEY_REMOTE_LEN 6
#define KEY_VERB_LEN   4
#define KEY_CA_LEN     2

#define VPN_DISCONNECT     0
#define VPN_CONNECTED      1
#define VPN_CONNECTING     3
#define VPN_RECONNECTING   4

#define DEFAULT_VERSION     "2.2.2"

//#define CHECK_OVPN      1   //检查openvpn的配置文件

class VpnParams:public QObject
{
    Q_OBJECT
public:
    VpnParams(QObject *parent = 0);
    ~VpnParams();
public:
    QString work_path;
    QString exe;            //"openvpn.exe"
    QString exe_version;    //2.2.2
    QString cmd_line;       //"openvpn.exe"

    //QString remote;
    QString tmp_dir;
    QString conf_file;
    QString conf_dir;

    QString log_file;       //sslvpn.log
    bool debug;

    //QString proto;

    bool auto_connect;
    bool remember_thumb;
    bool is_rsa;
    QString thumb;

    QString last_line;
    int current_status;
    int last_status;

    int failed_psw;
    int failed_psw_attempts;

    bool restart;

    QMap <QString,QString > Status_map;
    /*
    QString IP;
    */

    QMap <QString,QString > ovpn_map;

    //http action read from config
    QString current_version;
    QString check_version;      //gui_version from ini file
    QString down_exe;
    QString verify;
    QString os_type;

    //config md5
    QString config_md5;

public:
    void setStatus(QString key,QString Value);
    QString getStatus(QString key);

    QString Translate_line(QString msg);
    void ChangeStatus(int status);

//add for http request
    QString check_version_url();
    QString updata_url();
    QString verify_url_perfix();
    QString add_verify_item(QString item,QString value);

    //base 64中有加号
    QUrl verify_url_perfix_encode();
    QUrl add_verify_item_encode(QUrl url, QString item, QString value);
public:

};


#endif // PUBLIC_DEF_H
