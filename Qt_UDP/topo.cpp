#include "topo.h"
#include "ui_topo.h"
#include "comm.h"

topo::topo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::topo)
{
    ui->setupUi(this);
}

topo::~topo()
{
    delete ui;
}

void topo::paintEvent(QPaintEvent *)
{
    QPen pen;
    QPainter painter(this);
    QFontMetrics fm=painter.fontMetrics();//获取字体长宽
    //画主节点
    QPointF Master;
    Master.setX(width()/2);
    Master.setY(height()/2);
    painter.setBrush(QBrush(Qt::red));
    painter.drawEllipse(Master,25,25); //有多种画法，这里是指定圆心和x、y轴的半径;当x、y相等时就是画圆了
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter, "M");
    //画子节点

    QPointF Slave[16];
    QLineF Line[16];
    QString txt[16];
    QString rssi[16];
    for (int i=0; i<16; i++)
    {
        Slave[i].setX(width()/2+std::min(width(),height())/10*4*cos(i*22.5*M_PI/180));
        Slave[i].setY(height()/2+std::min(width(),height())/10*4*sin(i*22.5*M_PI/180));

        Line[i].setLine(width()/2+25*cos(i*22.5*M_PI/180),height()/2+25*sin(i*22.5*M_PI/180),Slave[i].x(),Slave[i].y());
        painter.setPen(Qt::black);
        painter.drawLine(Line[i]);
        if(i==7){
            painter.setBrush(QBrush(Qt::green));
            painter.drawEllipse(Slave[i],15,15);
        }else{
            painter.setBrush(QBrush(Qt::gray));
            painter.drawEllipse(Slave[i],15,15);
        }
        txt[i] = "S"+QString::number(i+1);
        rssi[i]="-"+QString::number(i+1)+"dBm";
        painter.setPen(Qt::black);
        painter.drawText(Slave[i].x()-fm.horizontalAdvance(txt[i])/2,Slave[i].y()+fm.ascent()/2-fm.descent()/2, txt[i]);
        painter.drawText(width()/2+std::min(width(),height())/10*2*cos(i*22.5*M_PI/180),height()/2+std::min(width(),height())/10*2*sin(i*22.5*M_PI/180), rssi[i]);
    }

    //显示Rssi


    //1、画字
    //painter.setPen(Qt::blue);
//    painter.setPen(Qt::blue);
//    painter.setFont(QFont("Arial", 50));
//    painter.drawText(rect(), Qt::AlignCenter, "Qt");
    //2、画多边形
//    QRectF rectangle(100.0, 20.0, 80.0, 60.0);
//    int startAngle = 30 * 16;
//    int spanAngle = 120 * 16;

//    painter.drawArc(rectangle, startAngle, spanAngle);

//    static const QPointF points[4] = {
//        QPointF(10.0, 80.0),
//        QPointF(20.0, 10.0),
//        QPointF(80.0, 30.0),
//        QPointF(90.0, 70.0)
//    };
//    painter.drawConvexPolygon(points, 4);
    //3、画圆
//    QRectF rectangle1(450.0, 350.0, 50.0, 50.0);
//    //QPainter painter(this);
//    painter.drawEllipse(rectangle1);
    //4、画线
//    QLineF line(10.0, 80.0, 90.0, 20.0);
//    //QPainter(this);
//    //painter.setPen(QColor(Qt::red));
//    painter.drawLine(line);
//    //5、画
//    static const QPointF points1[3] = {
//        QPointF(110.0, 180.0),
//        QPointF(120.0, 110.0),
//        QPointF(180.0, 130.0),
//    };
//    painter.drawPolyline(points1, 3);
//    //6、画矩形
//    painter.setPen(Qt::red);
//    QRectF rectangle2(200.0, 20.0, 80.0, 60.0);
//    painter.drawRect(rectangle2);
//    //7、画圆角矩形
//    QRectF rectangle3(200.0, 120.0, 80.0, 60.0);
//    painter.drawRoundedRect(rectangle3, 20.0, 15.0);
    //8、画多个点
//    QPointF pointf[10];
//    pen.setWidth(6);
//    painter.setPen(pen); // 设置画笔
//    for (int i=0; i<10; ++i)
//    {
//        pointf[i].setX(2.0+i*10.0);
//        pointf[i].setY(130.0);
//    }
//    painter.drawPoints(pointf, 10);
    //9、画单个直线
//    pen.setWidth(5);
//    pen.setColor(Qt::red); // 设置画笔为黄色
//    painter.setPen(pen); // 设置画笔
//    painter.drawLine(rect().topLeft(), rect().bottomRight());
    //10、画点
//    pen.setWidth(6);
//    pen.setColor(Qt::green); // 设置画笔为黄色
//    painter.setPen(pen); // 设置画笔
//    painter.drawPoint(200,200);
    //11、画圆点
//    painter.setPen(QColor(Qt::blue));
//    painter.setBrush(QBrush(Qt::yellow));
//    painter.drawEllipse(50,150,15,15);
}

void topo::on_comm_bt_clicked()
{
    this->close();
    Comm *comm1=new Comm();
    comm1->show();
}

