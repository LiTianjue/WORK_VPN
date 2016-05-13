#ifndef HANDLE_JSON_H
#define HANDLE_JSON_H

#include <QObject>
#include <QXmlStreamReader>
#include <QFile>
#include <QByteArray>

#include <QDebug>

#include <QSysInfo>
#include <QFileDialog>
#include <QDesktopServices>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include "http.h"



class CHandle_JSON:public QObject
{
    Q_OBJECT
public:
    CHandle_JSON(QObject *parent = 0);
    ~CHandle_JSON();

private:
    //错误
    QJsonParseError jsonError;
    QFile json_file;
    QByteArray json_arry;
public:
    QJsonDocument json_doc;

public:
    bool load_json(QString filename);
    bool load_json(QByteArray data);

    bool get_bool(QString key);
    QString get_string(QString key);
    int get_num(QString key);

    //void reload();
    //QByteArray& GetNode(QString nodename);

private:
    //QByteArray xml_context;

public:
    QByteArray _rt;
    QString err_string;
};


class Check_Version:public CHandle_JSON
{
public:
    Check_Version();
    ~Check_Version();
public:
    virtual bool check(QString url);
    QString get_name();
    QString get_version();
    int get_length();
    bool get_flag();

};

class Verify_User:public Check_Version
{
public:
    Verify_User();
    ~Verify_User();
public:
    virtual bool check(QString url);
    bool sava_json(QString key,QString Path);



};







#endif // HANDLE_JSON_H

