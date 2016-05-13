#ifndef MYHELPER_H
#define MYHELPER_H

#include <QDesktopWidget>
#include <QFile>
#include <QTextCodec>
#include <QMessageBox>
#include <QAbstractButton>
#include <QCoreApplication>
#include <QFileDialog>
#include <QTime>
#include <QProcess>
#include <QDir>
#include <QSound>
#include <QApplication>
#include <QStyleFactory>
#include <QInputDialog>
#include <QDebug>
#include <QDesktopServices>
#include <QCryptographicHash>

/*描述：辅助类，包含对话框，中文显示，文件处理等
 *作者：刘典武
 *时间：2013-12-12
*/
class myHelper:public QObject
{
public:

    //设置全局为plastique样式
    static void SetStyle()
    {
        QApplication::setStyle(QStyleFactory::create("Plastique"));
    }

    //设置编码为GB2312
    static void SetGB2312Code()
    {
        QTextCodec *codec=QTextCodec::codecForName("GB2312");
        QTextCodec::setCodecForLocale(codec);
        /*
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
        */
    }

    //设置编码为UTF8
    static void SetUTF8Code()
    {
        QTextCodec *codec=QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);
        /*
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
        */
    }

    //显示信息框，仅确定按钮
    static void ShowMessageBoxInfo(QString info)
    {
        QMessageBox msg;
        msg.setStyleSheet("font:12pt '宋体'");
        msg.setWindowTitle("提示");
        msg.setText(info);
        msg.setIcon(QMessageBox::Information);
        msg.addButton("确定",QMessageBox::ActionRole);
        msg.exec();
    }

    static void ShowMessageBoxInfoX(QString info)
    {
        QMessageBox::information(0,"提示",info,QMessageBox::Ok);
    }

    //显示错误框，仅确定按钮
    static void ShowMessageBoxError(QString info)
    {
        QMessageBox msg;
        msg.setStyleSheet("font:12pt '宋体'");
        msg.setWindowTitle("错误");
        msg.setText(info);
        msg.setIcon(QMessageBox::Critical);
        msg.addButton("确定",QMessageBox::ActionRole);
        msg.exec();
    }

    static void ShowMessageBoxErrorX(QString info)
    {
        QMessageBox::critical(0,"错误",info,QMessageBox::Ok);
    }

    //显示询问框，确定和取消按钮
    static int ShowMessageBoxQuesion(QString info)
    {
        QMessageBox msg;
        msg.setStyleSheet("font:12pt '宋体'");
        msg.setWindowTitle("询问");
        msg.setText(info);
        msg.setIcon(QMessageBox::Question);
        msg.addButton("确定",QMessageBox::ActionRole);
        msg.addButton("取消",QMessageBox::RejectRole);

        return msg.exec();
    }

    static int ShowMessageBoxQuesionX(QString info)
    {
        return QMessageBox::question(0,"询问",info,QMessageBox::Yes|QMessageBox::No);
    }

    //显示标准输入框
    static QString ShowInputBox(QWidget *frm,QString info)
    {
        bool ok;
        return QInputDialog::getText(frm,"提示",info,QLineEdit::Password,"",&ok);
    }

    //16进制字符串转字节数组
    static QByteArray HexStrToByteArray(QString str)
    {
        QByteArray senddata;
        /*
        int hexdata,lowhexdata;
        int hexdatalen = 0;
        int len = str.length();
        senddata.resize(len/2);
        char lstr,hstr;
        for(int i=0; i<len; )
        {
            hstr=str[i].toAscii();
            if(hstr == ' ')
            {
                i++;
                continue;
            }
            i++;
            if(i >= len)
                break;
            lstr = str[i].toAscii();
            hexdata = ConvertHexChar(hstr);
            lowhexdata = ConvertHexChar(lstr);
            if((hexdata == 16) || (lowhexdata == 16))
                break;
            else
                hexdata = hexdata*16+lowhexdata;
            i++;
            senddata[hexdatalen] = (char)hexdata;
            hexdatalen++;
        }
        senddata.resize(hexdatalen);
        */

        //rewrite by andy start
        int hexdata,lowhexdata;
        int hexdatalen = 0;
        int len = str.length();
        senddata.resize(len/2);
        char lstr,hstr;
        QString tmp;
        for(int i=0; i<len; )
        {
            tmp = str[i];
            if(tmp == " ")
            {
                i++;
                continue;
            }

            //hstr=str[i];
            hexdata = tmp.toUInt(0,16);

            i++;
            if(i >= len)
                break;
            tmp = str[i];
            lowhexdata = tmp.toUInt(0,16);

            if((hexdata == 16) || (lowhexdata == 16))
                break;
            else
                hexdata = hexdata*16+lowhexdata;
            i++;
            senddata[hexdatalen] = (char)hexdata;
            hexdatalen++;
        }
        senddata.resize(hexdatalen);
        //rewrite by andy end
        return senddata;
    }

    static char ConvertHexChar(char ch)
    {
        if((ch >= '0') && (ch <= '9'))
            return ch-0x30;
        else if((ch >= 'A') && (ch <= 'F'))
            return ch-'A'+10;
        else if((ch >= 'a') && (ch <= 'f'))
            return ch-'a'+10;
        else return (-1);
    }

    //字节数组转16进制字符串
    static QString ByteArrayToHexStr(QByteArray data)
    {
        QString temp="";
        QString hex=data.toHex();
        for (int i=0;i<hex.length();i=i+2)
        {
            temp+=hex.mid(i,2)+" ";
        }
        return temp.trimmed().toUpper();
    }

    //16进制字符串转10进制
    static int StrHexToDecimal(QString strHex)
    {
        bool ok;
        return strHex.toInt(&ok,16);
    }

    //10进制字符串转10进制
    static int StrDecimalToDecimal(QString strDecimal)
    {
        bool ok;
        return strDecimal.toInt(&ok,10);
    }

    //2进制字符串转10进制
    static int StrBinToDecimal(QString strBin)
    {
        bool ok;
        return strBin.toInt(&ok,2);
    }

    //16进制字符串转2进制字符串
    static QString StrHexToStrBin(QString strHex)
    {
        uchar decimal=StrHexToDecimal(strHex);
        QString bin=QString::number(decimal,2);

        uchar len=bin.length();
        if (len<8)
        {
            for (int i=0;i<8-len;i++)
            {
                bin="0"+bin;
            }
        }

        return bin;
    }

    //10进制转2进制字符串一个字节
    static QString DecimalToStrBin1(int decimal)
    {
        QString bin=QString::number(decimal,2);

        uchar len=bin.length();
        if (len<=8)
        {
            for (int i=0;i<8-len;i++)
            {
                bin="0"+bin;
            }
        }

        return bin;
    }

    //10进制转2进制字符串两个字节
    static QString DecimalToStrBin2(int decimal)
    {
        QString bin=QString::number(decimal,2);

        uchar len=bin.length();
        if (len<=16)
        {
            for (int i=0;i<16-len;i++)
            {
                bin="0"+bin;
            }
        }

        return bin;
    }

    //计算校验码
    static uchar GetCheckCode(uchar data[],uchar len)
    {
        uchar temp=0;

        for (uchar i=0;i<len;i++)
        {
            temp+=data[i];
        }

        return temp%256;
    }

    //将溢出的char转为正确的uchar
    static uchar GetUChar(char data)
    {
        uchar temp;
        if (data>127)
        {
            temp=data+256;
        }
        else
        {
            temp=data;
        }
        return temp;
    }

    //延时
    static void Sleep(int sec)
    {
        QTime dieTime = QTime::currentTime().addMSecs(sec);
        while( QTime::currentTime() < dieTime )
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    //获取当前路径
    static QString GetCurrentPath()
    {
        return QString(QCoreApplication::applicationDirPath()+"/");
    }

    //播放声音
    static void PlaySound(QString soundName)
    {
        QSound::play(soundName);
    }

    //设置系统日期时间
    static void SetSystemDateTime(int year,int month,int day,int hour,int min,int sec)
    {
        QProcess p(0);

        p.start("cmd");
        p.waitForStarted();
        //p.write(QString("date %1-%2-%3\n").arg(year).arg(month).arg(day).toAscii());
       // p.write(QString("date %1-%2-%3\n").arg(year).arg(month).arg(day));
        p.closeWriteChannel();
        p.waitForFinished(1000);
        p.close();

        p.start("cmd");
        p.waitForStarted();
        //p.write(QString("time %1:%2:%3.00\n").arg(hour).arg(min).arg(sec).toAscii());
        //p.write(QString("time %1:%2:%3.00\n").arg(hour).arg(min).arg(sec));
        p.closeWriteChannel();
        p.waitForFinished(1000);
        p.close();
    }

    //窗体居中，并且只有关闭按钮，不能调整大小
    static void FormOnlyCloseInCenter(QWidget *frm)
    {
        //设置窗体居中
        QDesktopWidget desktop;
        int screenX=desktop.availableGeometry().width();
        int screenY=desktop.availableGeometry().height()-40;
        int frmX=frm->width();
        int frmY=frm->height();
        QPoint movePoint(screenX/2-frmX/2,screenY/2-frmY/2);
        frm->move(movePoint);

        //设置窗体不能调整大小
        frm->setFixedSize(frmX,frmY);

        //设置窗体只有最小化按钮
        frm->setWindowFlags(Qt::WindowCloseButtonHint);        
    }

    //窗体居中显示
    static void FormInCenter(QWidget *frm)
    {
        int screenX=qApp->desktop()->width();
        int screenY=qApp->desktop()->height()-60;
        int wndX=frm->width();
        int wndY=frm->height();
        QPoint movePoint((screenX-wndX)/2,(screenY-wndY)/2);
        frm->move(movePoint);
    }

    //窗体没有最大化按钮
    static void FormNoMaxButton(QWidget *frm)
    {
        frm->setWindowFlags(Qt::WindowMinimizeButtonHint);
    }

    //窗体只有关闭按钮
    static void FormOnlyCloseButton(QWidget *frm)
    {
        frm->setWindowFlags(Qt::WindowCloseButtonHint);
    }

    //窗体不能改变大小
    static void FormNotResize(QWidget *frm)
    {
        frm->setFixedSize(frm->width(),frm->height());
    }

    //获取桌面大小
    static QSize GetDesktopSize()
    {
        QDesktopWidget desktop;
        return QSize(desktop.availableGeometry().width(),desktop.availableGeometry().height());
    }

    //获取选择的文件
    static QString GetFileName(QString filter)
    {
        return QFileDialog::getOpenFileName(NULL,"选择文件",QCoreApplication::applicationDirPath(),filter);
    }

    //获取选择的文件集合
    static QStringList GetFileNames(QString filter)
    {
        return QFileDialog::getOpenFileNames(NULL,"选择文件",QCoreApplication::applicationDirPath(),filter);
    }

    //获取选择的目录
    static QString GetFolderName()
    {
        return QFileDialog::getExistingDirectory();;
    }

    //获取文件名，含拓展名
    static QString GetFileNameWithExtension(QString strFilePath)
    {
        QFileInfo fileInfo(strFilePath);
        return fileInfo.fileName();
    }

    //获取选择文件夹中的文件
    static QStringList GetFolderFileNames(QStringList filter)
    {
        QStringList fileList;
        QString strFolder = QFileDialog::getExistingDirectory();
        if (!strFolder.length()==0)
        {
            QDir myFolder(strFolder);

            if (myFolder.exists())
            {
                fileList= myFolder.entryList(filter);
            }
        }
        return fileList;
    }

    //文件夹是否存在
    static bool FolderIsExist(QString strFolder)
    {
        QDir tempFolder(strFolder);
        return tempFolder.exists();
    }

    //文件是否存在
    static bool FileIsExist(QString strFile)
    {
        QFile tempFile(strFile);
        return tempFile.exists();
    }

    //复制文件
    static bool CopyFile(QString sourceFile, QString targetFile)
    {
        if (FileIsExist(targetFile))
        {
            int ret=QMessageBox::information(NULL,"提示","文件已经存在，是否替换？",QMessageBox::Yes | QMessageBox::No);
            if (ret!=QMessageBox::Yes)
            {
                return false;
            }
        }
        return QFile::copy(sourceFile,targetFile);
    }

    //文件保存对话框 ,默认路径为桌面
    static QString GetSaveFile(QString title,QString defaultname)
    {
        QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + defaultname;
        QString filename = QFileDialog::getSaveFileName(NULL,title,desktop);
        return filename;
    }

    //add by andy
    //保存数据到文件
    static bool SaveFile(QString Path,QByteArray &data,int len)
    {
        QFile::remove(Path);
        QFile *file;
        file = new QFile(Path);
        if(!file->open(QIODevice::WriteOnly))
        {
            return false;
        }
        if(file->isOpen())
        {
            file->write(data.data(),len);

            file->flush();
            file->close();
            delete file;
            return true;
        }
    }
    static bool SaveFile(QString Path,QByteArray &data,int offset,int len)
    {
        QFile::remove(Path);
        QFile *file;
        file = new QFile(Path);
        if(!file->open(QIODevice::WriteOnly))
        {
            qDebug()<<"Open:"<<Path<<file->errorString();
            return false;
        }
        if(file->isOpen())
        {
            file->write(data.data()+offset,len);

            file->flush();
            file->close();
            delete file;
            return true;
        }
    }

    static bool SaveFile(QString Path,QByteArray &data)
    {
        QFile::remove(Path);
        QFile *file;
        file = new QFile(Path);
        if(!file->open(QIODevice::WriteOnly))
        {
            return false;
        }
        if(file->isOpen())
        {
            file->write(data.data(),data.size());

            file->flush();
            file->close();
            delete file;
            return true;
        }
    }
    //读取文件
    static QByteArray ReadFile(QString Path)
    {
        QByteArray rt;
        rt.clear();
        QFile *file;
        file = new QFile(Path);
        if(!file->open(QIODevice::ReadOnly))
        {
        }
        else if(file->isOpen())
        {
            rt= file->readAll();
        }
        delete file;
        return rt;
    }
    //获取文件的md5值
    static QByteArray GetFileMD5(QString Path)
    {

        QFile theFile(Path);
        if(!theFile.open(QIODevice::ReadOnly))
        {
            qDebug()<<"Open File"<< Path <<":"<<theFile.errorString();
            return NULL;
        }
        QByteArray ba;
        ba.clear();
        ba = QCryptographicHash::hash(theFile.readAll(),QCryptographicHash::Md5);
        theFile.close();
        return ba.toHex();
        //qDebug() << ba.toHex().constData();
    }

    //运行程序获取返回值
    static QString RunCmd(QString cmd)
    {
        QProcess process;
        process.start(cmd);
        if(process.waitForStarted())
        {
            process.waitForFinished();
            return process.readAllStandardOutput();
        }
        else
            return "";
    }
    //密码对话框输入密码
    static QString PasswdDlg(QString title,QString prop)
    {
        bool ok;
        QString psd ;

        psd = QInputDialog::getText(0,title,prop,QLineEdit::Password,"",&ok);
        if(ok)
        {
            return psd;
        }
        else
        {
            psd.clear();
            return psd;
        }
    }

    //获取操作系统位数
    static QString GetSysVersion()
    {
       if(QSysInfo::currentCpuArchitecture().contains("i386"))
       {
             //ui->textBrowser->append("您使用的是32位系统");
           return "x86";
       }
       if(QSysInfo::currentCpuArchitecture().contains("64"))
       {
             //ui->textBrowser->append("您使用的是64位系统");
           return "x64";
       }
    }

};

#endif // MYHELPER_H
