#ifndef COMM_H
#define COMM_H

#include <QWidget>

namespace Ui {
class Comm;
}

class Comm : public QWidget
{
    Q_OBJECT

public:
    explicit Comm(QWidget *parent = nullptr);
    ~Comm();

private slots:

    void on_config_bt_clicked();


    void on_map_bt_clicked();

    void on_topo_bt_clicked();

private:
    Ui::Comm *ui;
};

#endif // COMM_H
