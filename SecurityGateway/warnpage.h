#ifndef WARNPAGE_H
#define WARNPAGE_H

#include <QWidget>

namespace Ui {
class WarnPage;
}

class WarnPage : public QWidget
{
    Q_OBJECT

public:
    explicit WarnPage(QWidget *parent = 0);
    ~WarnPage();

    void setBackGround();

private slots:
    void on_pushButton_clicked();

private:
    Ui::WarnPage *ui;
};

#endif // WARNPAGE_H
