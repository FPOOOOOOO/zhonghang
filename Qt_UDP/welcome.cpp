#include "welcome.h"
#include "ui_welcome.h"
#include "user.h"
#include "comm.h"

welcome::welcome(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::welcome)
{
    ui->setupUi(this);
}

welcome::~welcome()
{
    delete ui;
}

/*
 * login
 */

void welcome::on_login_bt_clicked()
{
    this->close();
    Comm *comm1 =new Comm();
    comm1->show();
}

