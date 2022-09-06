#ifndef TOPO_H
#define TOPO_H

#include <QWidget>
#include <QPainter>
#include <QPen>

namespace Ui {
class topo;
}

class topo : public QWidget
{
    Q_OBJECT

public:
    explicit topo(QWidget *parent = nullptr);
    ~topo();

public:
     void paintEvent(QPaintEvent *);

private slots:
     void on_comm_bt_clicked();

private:
    Ui::topo *ui;
};

#endif // TOPO_H
