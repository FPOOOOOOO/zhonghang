#include "widget.h"
#include "welcome.h"
#include "comm.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    setbuf(stdout, NULL);
//    Widget w;
//    w.show();
    welcome w;
    w.show();
//    Comm c;
//    c.show();

    return a.exec();
}
