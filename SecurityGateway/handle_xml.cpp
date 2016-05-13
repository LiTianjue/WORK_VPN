#include "handle_xml.h"
#include <QDebug>

CHandle_XML::CHandle_XML(QObject *parent)
{
    xml = new QXmlStreamReader();
}

CHandle_XML::~CHandle_XML()
{
    delete xml;
    if(xml_file.isOpen())
        xml_file.close();

}

bool CHandle_XML::load_xml(QString filename)
{
    //QFile file(filename);
    qDebug()<<"打开文件"<<filename<<endl;
    xml_file.setFileName(filename);
    if(!xml_file.open(QFile::ReadOnly|QFile::Text))
    {
        err_string = xml_file.errorString();
        return false;
    }
    qDebug()<<"打开文件"<<filename<<"成功"<<endl;
    xml->setDevice(&xml_file);
    //xml->addData();
    return true;
}

bool CHandle_XML::load_xml(QByteArray data)
{
    xml->addData(data);
    xml_context = data;
    return true;
}

void CHandle_XML::reload()
{
    xml->clear();
    if(xml_file.isOpen())
    {
        xml_file.seek(0);
        xml->setDevice(&xml_file);
    }else
    {
        xml->addData(xml_context);
    }

}

//解析一个节点
QByteArray &CHandle_XML::GetNode(QString nodename)
{
    _rt.clear();

    while(!xml->atEnd() && !xml->hasError())
    {
        QXmlStreamReader::TokenType token = xml->readNext();

        //获取的是 StartDocument
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }


        if(token == QXmlStreamReader::StartElement)
        {
            //ui->textBrowser->append("read Element");

            if(xml->name() != nodename)
            {
                continue;
            }
            else
            {
                //这里要不要转换一下
                //_rt = xml->readElementText().toLatin1();
                QString data = xml->readElementText();
                _rt = data.toLocal8Bit();
                break;
            }

        }
    }

    reload();

    return _rt;
}
