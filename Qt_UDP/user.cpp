#include "user.h"
#include "ui_user.h"
#include "comm.h"

#include <QDebug>
#include "pcap.h"

//从这里产生的全局变量在这里声明
QString user::UDPrecv;

user::user(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::user)
{
    ui->setupUi(this);
    udpSocket = new QUdpSocket(this);
    //界面初始化 显示网口
    //Obtain NetGate
    pcap_if_t *allDevs;
    pcap_if_t *temp;
    int i=0;
    char errbuf[PCAP_ERRBUF_SIZE];
    /* Retrieve the device list */
    if(pcap_findalldevs(&allDevs, errbuf) == -1){

        qDebug() << "error:" << errbuf;
    }

    /* Print the list */
    for(temp=allDevs; temp; temp=temp->next)
    {
        printf("%d. %s",++i,(temp->name));
        if (temp->description){
            printf(" (%s)\n",(temp->description));
            ui->NetGate->addItem(temp->description);
        }
        else
            printf("(No description available)\n");

    }

    pcap_freealldevs(allDevs);
}

user::~user()
{
    delete ui;
}

void user::on_comm_bt_clicked()
{
    printf("I am Jumping");
    this->close();
    Comm *comm1=new Comm();
    comm1->show();
}


void user::on_ETH_bt_clicked()
{
//    printf("I am IN NO1 bt");
//    this->close();
//    Comm *comm1=new Comm();
//    comm1->show();
    QString devName;
    //Obtain NetGate
    pcap_if_t *allDevs;
    pcap_if_t *temp;
    int i=0;
    char errbuf[PCAP_ERRBUF_SIZE];
    /* Retrieve the device list */
    if(pcap_findalldevs(&allDevs, errbuf) == -1){

        qDebug() << "error:" << errbuf;
    }

    /* Print the list */
    for(temp=allDevs; temp; temp=temp->next)
    {
        printf("%d. %s",++i,(temp->name));
        if (temp->description){
            printf(" (%s)\n",(temp->description));
            if(!QString::compare(temp->description,ui->NetGate->currentText(),Qt::CaseInsensitive)){
                devName=temp->name;
            }
        }
        else
            printf("(No description available)\n");

    }

    printf("Choose: %s \n",(ui->NetGate->currentText()).toStdString().c_str());
    //devName = allDevs->next->name;
    printf("NetGateName is:  %s\n",devName.toStdString().c_str());

    pcap_freealldevs(allDevs);


    //Open Netgate
    pcap_t *fp;
    u_char packet[100];

    if((fp= pcap_open_live(devName.toStdString().c_str(), 100, 1, 1000, errbuf)) == NULL){
        qDebug() << "error:" << errbuf;
        printf("已经打开");
    }

    packet[0]=1;
    packet[1]=1;
    packet[2]=1;
    packet[3]=1;
    packet[4]=1;
    packet[5]=1;

    packet[6]=2;
    packet[7]=2;
    packet[8]=2;
    packet[9]=2;
    packet[10]=2;
    packet[11]=2;

    for(int i = 12; i < 100; i++){

        packet[i]=0xFF;
    }

    if(pcap_sendpacket(fp, packet, 100) != 0){

        qDebug() << "send error:" << pcap_geterr(fp);
    }

    //pcap_sendpacket(fp, packet, 100);

    qDebug() << "over";
}

void user::on_ETHtest_clicked()
{
   // printf("指令已下发");
    QString buf;
    if(ui->s1->isChecked()==true){
        buf = QString::fromLocal8Bit("S1已下发");
        ui->s1->setCheckState(Qt::Unchecked);
    }
    else{
        buf = QString::fromLocal8Bit("指令已下发");
    }

    ui->plainTextEdit->appendPlainText(buf);
    UDPrecv=buf;
}
