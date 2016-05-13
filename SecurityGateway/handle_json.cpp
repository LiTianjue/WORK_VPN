#include "handle_json.h"
#include "myhelper.h"

#define CHECK_TIMEOUT   5*1000
#define VERIFY_TIMEOUT  40*1000


CHandle_JSON::CHandle_JSON(QObject *parent)
{

}

CHandle_JSON::~CHandle_JSON()
{

}

bool CHandle_JSON::load_json(QString filename)
{
    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open json file.");
        err_string = "打开json文件失败："+loadFile.errorString();
        return false;
    }
    else
    {
        qDebug() << "Open json Success" ;
    }

    //QJsonParseError jsonError;//Qt5新类

    QByteArray saveData = loadFile.readAll();

    loadFile.close();
    //QJsonDocument json(QJsonDocument::fromJson(saveData,&jsonError));
    json_doc = QJsonDocument::fromJson(saveData,&jsonError);


    if(jsonError.error == QJsonParseError::NoError)//Qt5新类
    {
        qDebug() << "解析成功" <<endl;

    QString name = json_doc.object()["name"].toString();

    qDebug() << "name :" << name <<endl;
       return true;
    }
    else
    {
        qDebug() << "Parse Fail : " <<jsonError.errorString();
        err_string = "解析失败：" + jsonError.errorString();
        return false ;
    }
}

bool CHandle_JSON::load_json(QByteArray data)
{
    json_doc = QJsonDocument::fromJson(data,&jsonError);

    if(jsonError.error == QJsonParseError::NoError)//Qt5新类
    {
       return true;
    }
    else
    {
        qDebug() << "Parse Fail : " <<jsonError.errorString();
        err_string = "解析失败：" + jsonError.errorString();
        return false ;
    }
}

bool CHandle_JSON::get_bool(QString key)
{
    bool ret = json_doc.object()[key].toBool();
    return ret;
}

QString CHandle_JSON::get_string(QString key)
{
    QString ret  = json_doc.object()[key].toString();
    return ret;
}

int CHandle_JSON::get_num(QString key)
{
    int ret  = json_doc.object()[key].toInt();
    return ret;
}


Check_Version::Check_Version()
{

}

Check_Version::~Check_Version()
{

}

bool Check_Version::check(QString url)
{
    Http http;
    QByteArray& array = http.GetDownLoadUrl(url,CHECK_TIMEOUT);
    if(array.isEmpty())
    {
        err_string = "检查更新失败:"+ http.err_string;
        return false;
    }
    //qDebug() << "Get Array:" << array;

    if(!load_json(array))
    {
        err_string = "无法解析服务器返回信息";
        return false;
    }
    return true;
}

QString Check_Version::get_name()
{
   QString ret = get_string("name");
   return ret;
}

QString Check_Version::get_version()
{
   QString ret = get_string("version");
   return ret;

}

int Check_Version::get_length()
{
   int ret = get_num("length");
   return ret;
}

bool Check_Version::get_flag()
{
   bool ret = get_bool("flag");
   return ret;
}







Verify_User::Verify_User()
{

}

Verify_User::~Verify_User()
{

}

bool Verify_User::check(QString url)
{
    Http http;
    QByteArray& array = http.GetDownLoadUrl(url,VERIFY_TIMEOUT);

    if(array.isEmpty())
    {
        err_string = "身份认证失败："+http.err_string;
        return false;
    }
    //delect later
    //qDebug() << "返回信息:" <<array;
    //qDebug() << "长度:" <<array.size();
    /*
    if(!myHelper::SaveFile("verify_ack.json",array,0,array.size()))
    {
        qDebug()<<"保存认证信息失败" <<endl;
    }
    */

    if(!load_json(array))
    {
        err_string = "无法解析服务器的返回信息";
        return false;
    }

    return true;
}

bool Verify_User::sava_json(QString key, QString Path)
{
    QString context = get_string(key);

    int size = context.simplified().size();
    //qDebug() << "Get["<<key<<"size:" <<QString(size) << "]:"<< context;

    QByteArray text = QByteArray::fromBase64(context.toLatin1());

    qDebug()<<"Save " << key << text.size();

    if(myHelper::SaveFile(Path,text))
        return true;
    return false;
}



















