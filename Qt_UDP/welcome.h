#ifndef WELCOME_H
#define WELCOME_H

#include <QWidget>

namespace Ui {
class welcome;
}

class welcome : public QWidget
{
    Q_OBJECT

public:
    explicit welcome(QWidget *parent = nullptr);
    ~welcome();

private slots:
    void on_login_bt_clicked();

private:
    Ui::welcome *ui;
};

#endif // WELCOME_H
