#ifndef USER_H
#define USER_H

#include <QWidget>
#include <QUdpSocket>
#include <QString>
#include <QMessageBox>
#include <QHostAddress>

//FUCK YOU！

namespace Ui {
class user;
}

class user : public QWidget
{
    Q_OBJECT

public:
    explicit user(QWidget *parent = nullptr);
    ~user();

    QUdpSocket *udpSocket;
    //Global
    static QString UDPrecv;

private slots:
    void on_comm_bt_clicked();

    void on_ETH_bt_clicked();

    void on_ETHtest_clicked();
    void on_UDPOpen_clicked();


private:
    Ui::user *ui;
};

#endif // USER_H
