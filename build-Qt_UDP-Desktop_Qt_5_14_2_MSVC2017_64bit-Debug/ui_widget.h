/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QGridLayout *gridLayout_2;
    QSpacerItem *verticalSpacer_7;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_4;
    QSpacerItem *horizontalSpacer_3;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QPlainTextEdit *recvplainTextEdit;
    QSpacerItem *verticalSpacer_4;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label;
    QLineEdit *locaportlineEdit_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QLineEdit *aimportlineEdit_3;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QLineEdit *aimiplineEdit_4;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *verticalSpacer_3;
    QHBoxLayout *horizontalLayout_6;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *sendlineEdit;
    QSpacerItem *verticalSpacer_5;
    QHBoxLayout *horizontalLayout_7;
    QPushButton *openpushButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *closepushButton_2;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *pushButton_3;
    QSpacerItem *horizontalSpacer_6;
    QPushButton *pushButton;
    QSpacerItem *verticalSpacer_6;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(1024, 600);
        Widget->setMinimumSize(QSize(1024, 600));
        Widget->setMaximumSize(QSize(1024, 600));
        Widget->setBaseSize(QSize(1024, 600));
        gridLayout_2 = new QGridLayout(Widget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        verticalSpacer_7 = new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Fixed);

        gridLayout_2->addItem(verticalSpacer_7, 2, 2, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(250, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 1, 0, 1, 1);

        label_4 = new QLabel(Widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        QFont font;
        font.setPointSize(14);
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_4, 2, 3, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(250, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_3, 1, 3, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(Widget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        recvplainTextEdit = new QPlainTextEdit(groupBox);
        recvplainTextEdit->setObjectName(QString::fromUtf8("recvplainTextEdit"));
        recvplainTextEdit->setReadOnly(true);

        horizontalLayout->addWidget(recvplainTextEdit);


        verticalLayout_2->addWidget(groupBox);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_4);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label = new QLabel(Widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_5->addWidget(label);

        locaportlineEdit_2 = new QLineEdit(Widget);
        locaportlineEdit_2->setObjectName(QString::fromUtf8("locaportlineEdit_2"));

        horizontalLayout_5->addWidget(locaportlineEdit_2);


        gridLayout->addLayout(horizontalLayout_5, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(Widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        aimportlineEdit_3 = new QLineEdit(Widget);
        aimportlineEdit_3->setObjectName(QString::fromUtf8("aimportlineEdit_3"));

        horizontalLayout_3->addWidget(aimportlineEdit_3);


        verticalLayout->addLayout(horizontalLayout_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(Widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        aimiplineEdit_4 = new QLineEdit(Widget);
        aimiplineEdit_4->setObjectName(QString::fromUtf8("aimiplineEdit_4"));

        horizontalLayout_4->addWidget(aimiplineEdit_4);


        verticalLayout->addLayout(horizontalLayout_4);


        gridLayout->addLayout(verticalLayout, 0, 2, 2, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_2, 1, 0, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_5, 0, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_3);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        groupBox_2 = new QGroupBox(Widget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_2);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        sendlineEdit = new QLineEdit(groupBox_2);
        sendlineEdit->setObjectName(QString::fromUtf8("sendlineEdit"));

        horizontalLayout_2->addWidget(sendlineEdit);


        horizontalLayout_6->addWidget(groupBox_2);


        verticalLayout_2->addLayout(horizontalLayout_6);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_5);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        openpushButton = new QPushButton(Widget);
        openpushButton->setObjectName(QString::fromUtf8("openpushButton"));

        horizontalLayout_7->addWidget(openpushButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer);

        closepushButton_2 = new QPushButton(Widget);
        closepushButton_2->setObjectName(QString::fromUtf8("closepushButton_2"));

        horizontalLayout_7->addWidget(closepushButton_2);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_4);

        pushButton_3 = new QPushButton(Widget);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        horizontalLayout_7->addWidget(pushButton_3);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);

        pushButton = new QPushButton(Widget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_7->addWidget(pushButton);


        verticalLayout_2->addLayout(horizontalLayout_7);


        gridLayout_2->addLayout(verticalLayout_2, 1, 1, 1, 2);

        verticalSpacer_6 = new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Fixed);

        gridLayout_2->addItem(verticalSpacer_6, 0, 2, 1, 1);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "UDP", nullptr));
        label_4->setText(QCoreApplication::translate("Widget", "MCC\342\200\224\342\200\224ECDAV", nullptr));
        groupBox->setTitle(QCoreApplication::translate("Widget", "\346\216\245\346\224\266\347\252\227\345\217\243", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\346\234\254\345\234\260\347\253\257\345\217\243\357\274\232", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\347\233\256\346\240\207\347\253\257\345\217\243\357\274\232", nullptr));
        aimportlineEdit_3->setText(QString());
        label_3->setText(QCoreApplication::translate("Widget", "\347\233\256\346\240\207IP\357\274\232  ", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("Widget", "\345\217\221\351\200\201\347\253\257\347\252\227\345\217\243", nullptr));
        openpushButton->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200UDP", nullptr));
        closepushButton_2->setText(QCoreApplication::translate("Widget", "\345\205\263\351\227\255UDP", nullptr));
        pushButton_3->setText(QCoreApplication::translate("Widget", "\345\217\221\351\200\201\346\225\260\346\215\256", nullptr));
        pushButton->setText(QCoreApplication::translate("Widget", "\346\270\205\347\251\272\347\252\227\345\217\243", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
