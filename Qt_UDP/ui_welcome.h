/********************************************************************************
** Form generated from reading UI file 'welcome.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WELCOME_H
#define UI_WELCOME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_welcome
{
public:
    QLabel *label;
    QWidget *verticalLayoutWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QSpacerItem *verticalSpacer_3;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_3;
    QLineEdit *lineEdit_2;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *login_bt;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *forgot_bt;

    void setupUi(QWidget *welcome)
    {
        if (welcome->objectName().isEmpty())
            welcome->setObjectName(QString::fromUtf8("welcome"));
        welcome->resize(900, 700);
        label = new QLabel(welcome);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(280, 160, 301, 101));
        label->setPixmap(QPixmap(QString::fromUtf8(":/pic/welcome.png")));
        label->setScaledContents(true);
        verticalLayoutWidget = new QWidget(welcome);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(330, 310, 194, 81));
        gridLayout = new QGridLayout(verticalLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_4->addWidget(label_2);

        lineEdit = new QLineEdit(verticalLayoutWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout_4->addWidget(lineEdit);


        gridLayout->addLayout(horizontalLayout_4, 0, 0, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_3, 1, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_3 = new QLabel(verticalLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_5->addWidget(label_3);

        lineEdit_2 = new QLineEdit(verticalLayoutWidget);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));

        horizontalLayout_5->addWidget(lineEdit_2);


        gridLayout->addLayout(horizontalLayout_5, 2, 0, 1, 1);

        horizontalLayoutWidget_3 = new QWidget(welcome);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(330, 450, 210, 32));
        horizontalLayout_6 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        login_bt = new QPushButton(horizontalLayoutWidget_3);
        login_bt->setObjectName(QString::fromUtf8("login_bt"));

        horizontalLayout_6->addWidget(login_bt);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_3);

        forgot_bt = new QPushButton(horizontalLayoutWidget_3);
        forgot_bt->setObjectName(QString::fromUtf8("forgot_bt"));

        horizontalLayout_6->addWidget(forgot_bt);


        retranslateUi(welcome);

        QMetaObject::connectSlotsByName(welcome);
    } // setupUi

    void retranslateUi(QWidget *welcome)
    {
        welcome->setWindowTitle(QCoreApplication::translate("welcome", "\347\231\273\345\275\225\347\225\214\351\235\242", nullptr));
        label->setText(QString());
        label_2->setText(QCoreApplication::translate("welcome", " \347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        label_3->setText(QCoreApplication::translate("welcome", "   \345\257\206\347\240\201\357\274\232 ", nullptr));
        login_bt->setText(QCoreApplication::translate("welcome", "\347\231\273\345\275\225", nullptr));
        forgot_bt->setText(QCoreApplication::translate("welcome", "\345\277\230\350\256\260\345\257\206\347\240\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class welcome: public Ui_welcome {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WELCOME_H
