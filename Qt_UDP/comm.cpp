#include "comm.h"
#include "ui_comm.h"
#include "welcome.h"
#include "user.h"
#include "map.h"
#include "topo.h"

Comm::Comm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Comm)
{
    ui->setupUi(this);
    //ui->title->setStyleSheet("background-color:DeepSkyBlue;font-size:20px;color:white");设置样式，已经在.ui里实现了
}

Comm::~Comm()
{
    delete ui;
}


void Comm::on_config_bt_clicked()
{
    this->close();
    user *user1=new user();
    user1->show();
}


void Comm::on_map_bt_clicked()
{
    this->close();
    Map *map1=new Map();
    map1->show();
}


void Comm::on_topo_bt_clicked()
{
    this->close();
    topo *topo1=new topo();
    topo1->show();
}

