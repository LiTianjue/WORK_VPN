#include "tconfigfile.h"
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QMessageBox>
#include <QIODevice>

#include "public_def.h"

TConfigFile::TConfigFile(QString strFileDir)
{
    m_strFileDir = strFileDir;
    qDebug()<<"TConfigFile Init:"<< m_strFileDir;
    //openvpn的配置文件不是ini的，需要以列表的方式读取和修改
    //m_Strings = new QStringList;
    //如果需要日志这里要初始化日志文件

    m_Strings = new QStringList;
    LoadCfgFile(m_strFileDir);

}

TConfigFile::~TConfigFile()
{

}

void TConfigFile::SaveCfgFile(QString &Proto, QString &IP, QString &Port, QString &ca)
{
   if(!m_Strings)
   {
        //没有成功打开过配置文件
       ;
   }
   QString strSpace = " ";

    for(int i = 0; i < m_Strings->count();i++)
    {
        if(m_Strings->operator [](i) == "")
            continue;
        //qDebug()<<m_Strings->operator [](i);
        if(__findkey((m_Strings->operator [](i)),KEY_PROTO))
        {
            m_Strings->operator [](i) = KEY_PROTO +strSpace +Proto;
        }
        if(__findkey((m_Strings->operator [](i)),KEY_REMOTE))
        {
            m_Strings->operator [](i) = KEY_REMOTE +strSpace +IP + strSpace +Port;
        }
        if(__findkey((m_Strings->operator [](i)),KEY_CA))
        {
            m_Strings->operator [](i) = KEY_CA +strSpace +ca;
        }

    }

    QFile file(m_strFileDir);
    //QMessageBox::warning(0,"Open",m_strFileDir);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        //用这种方法不存在会创建一个新文件
        QMessageBox::warning(0,"Config ","Open vpn client Config file fail");
        return;
    }else
    {
        QTextStream textStream(&file);
        for(int i = 0 ;i < m_Strings->count();i++)
        {
          //  QMessageBox::warning(0,"写入",m_Strings->operator [](i));
           textStream<< m_Strings->operator [](i) <<" "<<"\r\n";
        }
        file.close();
    }


}

void TConfigFile::LoadCfgFile(QString strFileDir)
{

    m_Strings->clear();
    QFile file(strFileDir);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        //用这种方法不存在会创建一个新文件
        QMessageBox::warning(0,"Config ","Open vpn client Config file fail");
        return;
    }else
    {

        QTextStream textStream(&file);

        while(!textStream.atEnd())
            m_Strings->append(textStream.readLine());
        file.close();
        qDebug()<<m_Strings->count();
        qDebug()<<*(m_Strings);
    }
    for(int i = 0; i < m_Strings->count();i++)
    {
        if(m_Strings->operator [](i) == "")
            continue;
        //qDebug()<<m_Strings->operator [](i);
        if(__findkey((m_Strings->operator [](i)),KEY_PROTO))
        {
            m_strProto = (m_Strings->operator [](i)).mid(KEY_PROTO_LEN+1);
            qDebug()<<m_strProto;
        }
        if(__findkey((m_Strings->operator [](i)),KEY_REMOTE))
        {
            m_strVpn_server_ip = (m_Strings->operator [](i)).mid(KEY_REMOTE_LEN +1);
            int nPos = 0;
            nPos = m_strVpn_server_ip.indexOf(" ");
            m_strVpn_server_port = m_strVpn_server_ip.mid(nPos+1);
            m_strVpn_server_port = m_strVpn_server_port.trimmed();
            m_strVpn_server_ip = m_strVpn_server_ip.mid(0,nPos);
            qDebug()<<"ip :"<<m_strVpn_server_ip;
            qDebug()<<"port :"<<m_strVpn_server_port;
        }
        if(__findkey((m_Strings->operator [](i)),KEY_CA))
        {
            m_strCa = (m_Strings->operator [](i)).mid(KEY_CA_LEN +1);
            //qDebug()<<m_strCa;
        }

    }
}

void TConfigFile::ReLoadCfgFile()
{
    LoadCfgFile(m_strFileDir);
}

bool TConfigFile::__findkey(QString strSource, QString strKey)
{
    //qDebug()<<"in "<< strSource<< " Find:"<< strKey;
    int nPos = strSource.indexOf('#');
    if(nPos != -1)
        return false;
    nPos = strSource.indexOf(';');
    if(nPos != -1)
        return false;
    nPos = strSource.indexOf(strKey);
    if(nPos == -1)
        return false;

    qDebug()<<"find key :"<< strKey;
    return true;
}

















