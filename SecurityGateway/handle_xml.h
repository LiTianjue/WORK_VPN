#ifndef HANDLE_XML
#define HANDLE_XML

#include <QObject>
#include <QXmlStreamReader>
#include <QFile>
#include <QByteArray>



class CHandle_XML:public QObject
{
    Q_OBJECT
public:
    CHandle_XML(QObject *parent = 0);
    ~CHandle_XML();

private:
    QFile xml_file;
    QXmlStreamReader *xml;

public:
    bool load_xml(QString filename);
    bool load_xml(QByteArray data);

    void reload();
    QByteArray& GetNode(QString nodename);

private:
    QByteArray xml_context;

public:
    QByteArray _rt;
    QString err_string;
};


#endif // HANDLE_XML

