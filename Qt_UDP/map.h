#ifndef MAP_H
#define MAP_H

#include <QWidget>
#include <QWebEngineView>

namespace Ui {
class Map;
}

class Map : public QWidget
{
    Q_OBJECT

public:
    explicit Map(QWidget *parent = nullptr);
    ~Map();

private slots:
    void on_comm_bt_clicked();

    void on_marker_bt_clicked();

private:
    Ui::Map *ui;
};

#endif // MAP_H
