/********************************************************************************
** Form generated from reading UI file 'map.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAP_H
#define UI_MAP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Map
{
public:
    QLabel *label;
    QPushButton *comm_bt;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *pLayout;
    QLabel *label_2;
    QGroupBox *groupBox;
    QPlainTextEdit *plainTextEdit;
    QGroupBox *groupBox_2;
    QPlainTextEdit *plainTextEdit_2;
    QGroupBox *groupBox_3;
    QPlainTextEdit *plainTextEdit_3;
    QPushButton *marker_bt;

    void setupUi(QWidget *Map)
    {
        if (Map->objectName().isEmpty())
            Map->setObjectName(QString::fromUtf8("Map"));
        Map->resize(1200, 700);
        label = new QLabel(Map);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(700, 10, 231, 31));
        QFont font;
        font.setPointSize(20);
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        comm_bt = new QPushButton(Map);
        comm_bt->setObjectName(QString::fromUtf8("comm_bt"));
        comm_bt->setGeometry(QRect(870, 650, 141, 32));
        horizontalLayoutWidget = new QWidget(Map);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(410, 50, 761, 581));
        pLayout = new QHBoxLayout(horizontalLayoutWidget);
        pLayout->setObjectName(QString::fromUtf8("pLayout"));
        pLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(Map);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(110, 10, 181, 31));
        label_2->setFont(font);
        groupBox = new QGroupBox(Map);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(20, 60, 341, 191));
        plainTextEdit = new QPlainTextEdit(groupBox);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setGeometry(QRect(10, 20, 321, 161));
        groupBox_2 = new QGroupBox(Map);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(20, 260, 341, 191));
        plainTextEdit_2 = new QPlainTextEdit(groupBox_2);
        plainTextEdit_2->setObjectName(QString::fromUtf8("plainTextEdit_2"));
        plainTextEdit_2->setGeometry(QRect(10, 20, 321, 161));
        groupBox_3 = new QGroupBox(Map);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(20, 460, 341, 191));
        plainTextEdit_3 = new QPlainTextEdit(groupBox_3);
        plainTextEdit_3->setObjectName(QString::fromUtf8("plainTextEdit_3"));
        plainTextEdit_3->setGeometry(QRect(10, 20, 321, 161));
        marker_bt = new QPushButton(Map);
        marker_bt->setObjectName(QString::fromUtf8("marker_bt"));
        marker_bt->setGeometry(QRect(590, 650, 100, 32));

        retranslateUi(Map);

        QMetaObject::connectSlotsByName(Map);
    } // setupUi

    void retranslateUi(QWidget *Map)
    {
        Map->setWindowTitle(QCoreApplication::translate("Map", "\350\212\202\347\202\271\344\275\215\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("Map", "\350\212\202\347\202\271\344\275\215\347\275\256\345\234\260\345\233\276\346\230\276\347\244\272", nullptr));
        comm_bt->setText(QCoreApplication::translate("Map", "\347\234\213\346\235\277", nullptr));
        label_2->setText(QCoreApplication::translate("Map", "\350\212\202\347\202\271X\345\256\236\346\227\266\346\225\260\346\215\256", nullptr));
        groupBox->setTitle(QCoreApplication::translate("Map", "ETH", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("Map", "SPI", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("Map", "UART", nullptr));
        marker_bt->setText(QCoreApplication::translate("Map", "MapPoint", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Map: public Ui_Map {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAP_H
