#include "warnpage.h"
#include "ui_warnpage.h"
#include <QDesktopWidget>

WarnPage::WarnPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WarnPage)
{
    ui->setupUi(this);
    setBackGround();
}

WarnPage::~WarnPage()
{
    delete ui;
}

void WarnPage::setBackGround()
{
    QDesktopWidget* desktopWidget = QApplication::desktop();

    QRect deskRect = desktopWidget->availableGeometry();

    //QPixmap pixmap = QPixmap(":/image/images/warring.jpg").scaled(this->size());
    //QPixmap pixmap = QPixmap(":/image/images/warring.jpg").scaled(deskRect.size());
    QPixmap pixmap = QPixmap(":/warrning.jpg");
    QPalette palette(this->palette());
    palette.setBrush(this->backgroundRole(),
                     QBrush(pixmap.scaled(deskRect.size(),
                                          Qt::IgnoreAspectRatio,
                                          Qt::SmoothTransformation)));
    this->setPalette(palette);
    this->setAutoFillBackground(true);
}

void WarnPage::on_pushButton_clicked()
{
    this->close();
}
