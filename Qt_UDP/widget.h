#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QString>
#include <QMessageBox>
#include <QHostAddress>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();


    QUdpSocket *udpSocket;

private slots:
    void on_openpushButton_clicked();

    void readyRead_Slot();

    void on_closepushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H
