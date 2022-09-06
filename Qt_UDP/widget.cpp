#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
}

Widget::~Widget()
{
    delete ui;
}

/*
 * 点击打开按钮，绑定端口号到socket
 * 有了这部分，开启服务，然后输入端口号，就可以接收到数据了
 */
void Widget::on_openpushButton_clicked()
{
    /* 绑定本地端口号 */
    if(( udpSocket->bind(ui->locaportlineEdit_2->text().toInt()) ) == true)
    {
         QMessageBox::information(this, "提示", "UDP端口号绑定成功！");
    }else{
         QMessageBox::warning(this, "警告", "UDP端口号绑定失败！");
    }


    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readyRead_Slot()));
}

void Widget::readyRead_Slot()
{
    /* 等待接收到数据 */
    while (udpSocket->hasPendingDatagrams()) {

        QByteArray datagram;
        /* 调整数组大小一致 */
        datagram.resize(udpSocket->pendingDatagramSize());

        udpSocket->readDatagram(datagram.data(), datagram.size());      // 读取接收到的数据

        QString buf;
        buf = datagram.data();                                          // 转化成字符串类型
        ui->recvplainTextEdit->appendPlainText(buf);                    // 把内容显示在 接收窗口
    }
}

/* 关闭UDP服务 */
void Widget::on_closepushButton_2_clicked()
{
    udpSocket->close();
}

/* 清空接收窗口的内容 */
void Widget::on_pushButton_clicked()
{
    ui->recvplainTextEdit->clear();
}

/* 发送数据 */
void Widget::on_pushButton_3_clicked()
{
    quint16 port;                                       // 目标端口号地址

    QString sendbuff;                                   // 发送数据的缓存

    QHostAddress address;                               // 目标IP地址

    address.setAddress(ui->aimiplineEdit_4->text());

    sendbuff = ui->sendlineEdit->text();

    port = ui->aimportlineEdit_3->text().toUInt();

    udpSocket->writeDatagram(sendbuff.toLocal8Bit().data(), sendbuff.length(), address, port);
}
