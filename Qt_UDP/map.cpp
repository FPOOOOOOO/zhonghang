#include "map.h"
#include "ui_map.h"
#include "comm.h"
//#include <QHBoxLayout>
#include <QWebEngineView>

//全局变量 引入
#include "user.h"
//QString user::UDPrecv;

QWebEngineView *webView;

Map::Map(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Map)
{
    ui->setupUi(this);
    QHBoxLayout *pLayout=ui->pLayout;

//    QHBoxLayout *pLayout =new QHBoxLayout(this);
//    QWebEngineView *webView=new QWebEngineView(this);
    webView=new QWebEngineView(this);
    pLayout->addWidget(webView);
//Online part
    //webView->load(QUrl("qrc:/baidumaponline0620.html"));
//Offline part
    //QString htmlPath = QCoreApplication::applicationDirPath() + "/bmap-offline/baidumapoffline0624.html";
    QString htmlPath="C:/Users/fpo/Desktop/zhonghang/Upper/zhonghang-QTWIN/Qt_UDP/bmap-offline/baidumapoffline0624.html";
    //QString htmlPath="C:/Users/12849/Desktop/zhonghang/Qt_UDP/bmap-offline/baidumapoffline0624.html";

//    qDebug() << htmlPath;
//    QFile file(htmlPath);
//        if(!file.exists())
//            qDebug() << "html file is not exist";
    webView->load(QUrl("file:///" + htmlPath));
    //webView->load(QUrl("qrc:/bmap-offline/baidumapoffline0624.html"));
    //webView->load(QUrl(":/bmap-offline/baidumapoffline0624.html"));

//    webView->load(QUrl("http://www.baidu.com"));
//    double gpsLon=120.131654;
//    double gpsLat=30.26904;

//    QString jsStr="addMarker("+QString::number(gpsLon)+","+QString::number(gpsLat)+")";
//    webView->page()->runJavaScript(jsStr);
    ui->plainTextEdit->appendPlainText(user::UDPrecv);


}

Map::~Map()
{
    delete ui;
}

void Map::on_comm_bt_clicked()
{
    this->close();
    Comm *comm1=new Comm();
    comm1->show();
}


void Map::on_marker_bt_clicked()
{
    double gpsLon=120.131654;
    double gpsLat=30.26904;

//    double lon[4], lat[4];
//    lon[0] = 121.50866; lon[1] = 121.50863; lon[2] = 121.50899; lon[3] = 121.50902;
//    lat[0] = 31.28867; lat[1] = 31.28860; lat[2] = 31.28845; lat[3] = 31.28855;
//    for (int i = 0; i < 4; i++)
//    {
//        QString command = QString("movepoint(%1,%2)").arg(QString::number(lon[i], 'f', 6)).arg(QString::number(lat[i], 'f', 6));
//        webView->page()->runJavaScript(command);
//    }

//    QString addpoint = QString("addpoint(%1,%2)").arg(QString::number(gpsLon, 'f', 6)).arg(QString::number(gpsLat, 'f', 6));
    QString movepoint = QString("movepoint(%1,%2)").arg(QString::number(gpsLon, 'f', 6)).arg(QString::number(gpsLat, 'f', 6));
    webView->page()->runJavaScript(movepoint);
}

