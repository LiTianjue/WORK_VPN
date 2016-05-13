#ifndef DOWN_CONF
#define DOWN_CONF

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>

#include <QTimer>
#include <QByteArray>
#include <QThread>
#include <QEventLoop>
#include <QtNetwork/QNetworkReply>
#include <QTcpSocket>
#include "FaceInfoMan.h"
#include "handle_xml.h"

#define REQ_TIMEOUT  30*1000

/***********************************/
// comment by andy :
// 没有使用FaceInfoMan提供的通用接口，
// 因为在QT中调用会导致事件循环阻塞，程序假死
/***********************************/
class CDOWN_CONF:public QObject
{
    Q_OBJECT
public:
    CDOWN_CONF(QObject *parent = 0);
    ~CDOWN_CONF();

    bool TryConnect(QString IP,int port);

    QByteArray& GetResponse(const QString& _request);
    QByteArray& GetResponse(QByteArray& _request);
    
    bool SaveFile(QString Path,QByteArray& text);
    bool SaveConf(QString Path);
    
public:
    //bool load_xml(QByteArray xml);
    bool CheckResponse(QString Node,QString OK_String);
    bool Save_xml(QString perfixPath,QString nameNode,QString sizeNode,QString contextNode);
    QString Save_xml_tmp(QString tmp_dir,QString nameNode,QString sizeNode,QString contextNode);

public slots:
    //TCP 读写处理
    void ReadData();
    void ReadError(QAbstractSocket::SocketError);

    void handleTimeOut();	/*handle time out*/

private:
    int ReadHeader();
public:
    //构建请求正文
    QByteArray BuildContext_1(char *data,int len);
    //构建请求正文 ,在里面调用base64出错，不知道什么问题
    QByteArray BuildContext_2(QByteArray base64_data,int len);    //请求正文加上28字节头
    QByteArray BuildReq(QByteArray context);

private:
    QTcpSocket *tcpClient;
    //void Init();

public:
    CHandle_XML *xml;

private:
    QEventLoop*	qEvent;
    QTimer		*timeOut;

public:
    QString err_string;
    QByteArray	_rt;

// add for http & json

};
#endif // DOWN_CONF

