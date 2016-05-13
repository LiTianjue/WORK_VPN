#ifndef TCONFIGFILE_H
#define TCONFIGFILE_H
#include <QString>
#include <QStringList>

class TConfigFile
{
public:
    TConfigFile(QString strFileDir);
    ~TConfigFile();
    //载入配置文件
    void SaveCfgFile(QString& Proto,QString& IP, QString& Port,QString& ca);
    void LoadCfgFile(QString strFileDir);

    //add by andy reload cfg_file
    void ReLoadCfgFile();

    QString Get_proto();
    QString Get_vpn_server_ip();
    QString Get_vpn_server_port();

    QString m_strProto;
    QString m_strVpn_server_ip;
    QString m_strVpn_server_port;
    QString m_strCa;

private:
    QString m_strFileDir;
    QStringList *m_Strings;

    bool __findkey(QString strSource,QString strKey);
};

#endif // TCONFIGFILE_H
