/********************************************************************************
** Form generated from reading UI file 'topo.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TOPO_H
#define UI_TOPO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_topo
{
public:
    QLabel *label;
    QPushButton *comm_bt;

    void setupUi(QWidget *topo)
    {
        if (topo->objectName().isEmpty())
            topo->setObjectName(QString::fromUtf8("topo"));
        topo->resize(900, 700);
        label = new QLabel(topo);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(360, 10, 161, 31));
        QFont font;
        font.setPointSize(20);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);
        comm_bt = new QPushButton(topo);
        comm_bt->setObjectName(QString::fromUtf8("comm_bt"));
        comm_bt->setGeometry(QRect(400, 650, 100, 32));

        retranslateUi(topo);

        QMetaObject::connectSlotsByName(topo);
    } // setupUi

    void retranslateUi(QWidget *topo)
    {
        topo->setWindowTitle(QCoreApplication::translate("topo", "\346\213\223\346\211\221\345\233\276", nullptr));
        label->setText(QCoreApplication::translate("topo", "\350\212\202\347\202\271\346\213\223\346\211\221\345\233\276", nullptr));
        comm_bt->setText(QCoreApplication::translate("topo", "\347\234\213\346\235\277", nullptr));
    } // retranslateUi

};

namespace Ui {
    class topo: public Ui_topo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOPO_H
