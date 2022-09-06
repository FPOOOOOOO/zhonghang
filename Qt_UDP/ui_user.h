/********************************************************************************
** Form generated from reading UI file 'user.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USER_H
#define UI_USER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_user
{
public:
    QGroupBox *groupBox;
    QPlainTextEdit *plainTextEdit;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QLabel *label_2;
    QLineEdit *lineEdit_2;
    QPushButton *UDPOpen;
    QPushButton *UDPClose;
    QPushButton *ClearAll;
    QGroupBox *groupBox_2;
    QLabel *label_3;
    QLabel *MOS_2;
    QLabel *label_9;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_8;
    QLabel *label_7;
    QLabel *label_10;
    QLabel *Address_2;
    QLabel *MOS_3;
    QLabel *MAC;
    QLabel *SPIMOS_2;
    QLabel *SPI_rate_2;
    QLabel *Baudrate_2;
    QLabel *label_25;
    QLabel *Frequency_2;
    QGroupBox *groupBox_3;
    QLabel *label_17;
    QLabel *label_19;
    QLabel *label_20;
    QLabel *label_22;
    QLabel *label_23;
    QLabel *label_24;
    QPushButton *ETH_bt;
    QLabel *label_27;
    QComboBox *MOS;
    QComboBox *Address;
    QComboBox *Frequency;
    QComboBox *SPIMOS;
    QComboBox *SPI_rate;
    QComboBox *Baudrate;
    QCheckBox *s2;
    QCheckBox *s3;
    QCheckBox *s4;
    QCheckBox *s5;
    QCheckBox *s6;
    QCheckBox *s7;
    QCheckBox *s8;
    QCheckBox *s14;
    QCheckBox *s9;
    QCheckBox *s13;
    QCheckBox *s16;
    QCheckBox *s12;
    QCheckBox *s10;
    QCheckBox *s15;
    QCheckBox *s11;
    QCheckBox *s1;
    QLabel *label_18;
    QPushButton *comm_bt;
    QPushButton *ETHtest;

    void setupUi(QWidget *user)
    {
        if (user->objectName().isEmpty())
            user->setObjectName(QString::fromUtf8("user"));
        user->resize(900, 700);
        groupBox = new QGroupBox(user);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(190, 40, 541, 211));
        plainTextEdit = new QPlainTextEdit(groupBox);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setGeometry(QRect(10, 30, 521, 171));
        horizontalLayoutWidget = new QWidget(user);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(190, 260, 531, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(horizontalLayoutWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        label_2 = new QLabel(horizontalLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        lineEdit_2 = new QLineEdit(horizontalLayoutWidget);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));

        horizontalLayout->addWidget(lineEdit_2);

        UDPOpen = new QPushButton(horizontalLayoutWidget);
        UDPOpen->setObjectName(QString::fromUtf8("UDPOpen"));

        horizontalLayout->addWidget(UDPOpen);

        UDPClose = new QPushButton(horizontalLayoutWidget);
        UDPClose->setObjectName(QString::fromUtf8("UDPClose"));

        horizontalLayout->addWidget(UDPClose);

        ClearAll = new QPushButton(horizontalLayoutWidget);
        ClearAll->setObjectName(QString::fromUtf8("ClearAll"));

        horizontalLayout->addWidget(ClearAll);

        groupBox_2 = new QGroupBox(user);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(190, 310, 541, 111));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 30, 61, 16));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_3->setFont(font);
        label_3->setWordWrap(false);
        MOS_2 = new QLabel(groupBox_2);
        MOS_2->setObjectName(QString::fromUtf8("MOS_2"));
        MOS_2->setGeometry(QRect(80, 30, 61, 16));
        label_9 = new QLabel(groupBox_2);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(140, 30, 61, 16));
        label_9->setFont(font);
        label_9->setWordWrap(false);
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(270, 30, 61, 16));
        label_5->setFont(font);
        label_5->setWordWrap(false);
        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(410, 30, 61, 16));
        label_6->setFont(font);
        label_6->setWordWrap(false);
        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(10, 80, 61, 16));
        label_8->setFont(font);
        label_8->setWordWrap(false);
        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(140, 80, 61, 16));
        label_7->setFont(font);
        label_7->setWordWrap(false);
        label_10 = new QLabel(groupBox_2);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(270, 80, 91, 16));
        label_10->setFont(font);
        label_10->setWordWrap(false);
        Address_2 = new QLabel(groupBox_2);
        Address_2->setObjectName(QString::fromUtf8("Address_2"));
        Address_2->setGeometry(QRect(190, 30, 61, 16));
        MOS_3 = new QLabel(groupBox_2);
        MOS_3->setObjectName(QString::fromUtf8("MOS_3"));
        MOS_3->setGeometry(QRect(340, 30, 61, 16));
        MAC = new QLabel(groupBox_2);
        MAC->setObjectName(QString::fromUtf8("MAC"));
        MAC->setGeometry(QRect(470, 30, 61, 16));
        SPIMOS_2 = new QLabel(groupBox_2);
        SPIMOS_2->setObjectName(QString::fromUtf8("SPIMOS_2"));
        SPIMOS_2->setGeometry(QRect(80, 80, 61, 16));
        SPI_rate_2 = new QLabel(groupBox_2);
        SPI_rate_2->setObjectName(QString::fromUtf8("SPI_rate_2"));
        SPI_rate_2->setGeometry(QRect(200, 80, 61, 16));
        Baudrate_2 = new QLabel(groupBox_2);
        Baudrate_2->setObjectName(QString::fromUtf8("Baudrate_2"));
        Baudrate_2->setGeometry(QRect(360, 80, 61, 16));
        label_25 = new QLabel(groupBox_2);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setGeometry(QRect(410, 80, 61, 16));
        label_25->setFont(font);
        label_25->setWordWrap(false);
        Frequency_2 = new QLabel(groupBox_2);
        Frequency_2->setObjectName(QString::fromUtf8("Frequency_2"));
        Frequency_2->setGeometry(QRect(470, 80, 61, 16));
        groupBox_3 = new QGroupBox(user);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(190, 430, 541, 161));
        label_17 = new QLabel(groupBox_3);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setGeometry(QRect(10, 30, 61, 16));
        label_17->setFont(font);
        label_17->setWordWrap(false);
        label_19 = new QLabel(groupBox_3);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setGeometry(QRect(140, 30, 61, 16));
        label_19->setFont(font);
        label_19->setWordWrap(false);
        label_20 = new QLabel(groupBox_3);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setGeometry(QRect(270, 30, 61, 16));
        label_20->setFont(font);
        label_20->setWordWrap(false);
        label_22 = new QLabel(groupBox_3);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        label_22->setGeometry(QRect(10, 80, 61, 16));
        label_22->setFont(font);
        label_22->setWordWrap(false);
        label_23 = new QLabel(groupBox_3);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setGeometry(QRect(140, 80, 61, 16));
        label_23->setFont(font);
        label_23->setWordWrap(false);
        label_24 = new QLabel(groupBox_3);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        label_24->setGeometry(QRect(270, 80, 91, 16));
        label_24->setFont(font);
        label_24->setWordWrap(false);
        ETH_bt = new QPushButton(groupBox_3);
        ETH_bt->setObjectName(QString::fromUtf8("ETH_bt"));
        ETH_bt->setGeometry(QRect(440, 120, 83, 32));
        label_27 = new QLabel(groupBox_3);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setGeometry(QRect(10, 120, 41, 16));
        label_27->setFont(font);
        label_27->setWordWrap(false);
        MOS = new QComboBox(groupBox_3);
        MOS->addItem(QString());
        MOS->addItem(QString());
        MOS->setObjectName(QString::fromUtf8("MOS"));
        MOS->setGeometry(QRect(70, 30, 61, 22));
        Address = new QComboBox(groupBox_3);
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->addItem(QString());
        Address->setObjectName(QString::fromUtf8("Address"));
        Address->setGeometry(QRect(175, 30, 91, 22));
        Frequency = new QComboBox(groupBox_3);
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->addItem(QString());
        Frequency->setObjectName(QString::fromUtf8("Frequency"));
        Frequency->setGeometry(QRect(310, 30, 61, 22));
        SPIMOS = new QComboBox(groupBox_3);
        SPIMOS->addItem(QString());
        SPIMOS->addItem(QString());
        SPIMOS->setObjectName(QString::fromUtf8("SPIMOS"));
        SPIMOS->setGeometry(QRect(70, 80, 61, 22));
        SPI_rate = new QComboBox(groupBox_3);
        SPI_rate->addItem(QString());
        SPI_rate->addItem(QString());
        SPI_rate->setObjectName(QString::fromUtf8("SPI_rate"));
        SPI_rate->setGeometry(QRect(200, 80, 61, 22));
        Baudrate = new QComboBox(groupBox_3);
        Baudrate->addItem(QString());
        Baudrate->addItem(QString());
        Baudrate->setObjectName(QString::fromUtf8("Baudrate"));
        Baudrate->setGeometry(QRect(320, 80, 71, 22));
        s2 = new QCheckBox(groupBox_3);
        s2->setObjectName(QString::fromUtf8("s2"));
        s2->setGeometry(QRect(90, 110, 31, 18));
        s3 = new QCheckBox(groupBox_3);
        s3->setObjectName(QString::fromUtf8("s3"));
        s3->setGeometry(QRect(130, 110, 31, 18));
        s4 = new QCheckBox(groupBox_3);
        s4->setObjectName(QString::fromUtf8("s4"));
        s4->setGeometry(QRect(170, 110, 31, 18));
        s5 = new QCheckBox(groupBox_3);
        s5->setObjectName(QString::fromUtf8("s5"));
        s5->setGeometry(QRect(210, 110, 31, 18));
        s6 = new QCheckBox(groupBox_3);
        s6->setObjectName(QString::fromUtf8("s6"));
        s6->setGeometry(QRect(250, 110, 31, 18));
        s7 = new QCheckBox(groupBox_3);
        s7->setObjectName(QString::fromUtf8("s7"));
        s7->setGeometry(QRect(290, 110, 31, 18));
        s8 = new QCheckBox(groupBox_3);
        s8->setObjectName(QString::fromUtf8("s8"));
        s8->setGeometry(QRect(330, 110, 31, 18));
        s14 = new QCheckBox(groupBox_3);
        s14->setObjectName(QString::fromUtf8("s14"));
        s14->setGeometry(QRect(250, 130, 31, 18));
        s9 = new QCheckBox(groupBox_3);
        s9->setObjectName(QString::fromUtf8("s9"));
        s9->setGeometry(QRect(50, 130, 31, 18));
        s13 = new QCheckBox(groupBox_3);
        s13->setObjectName(QString::fromUtf8("s13"));
        s13->setGeometry(QRect(210, 130, 31, 18));
        s16 = new QCheckBox(groupBox_3);
        s16->setObjectName(QString::fromUtf8("s16"));
        s16->setGeometry(QRect(330, 130, 31, 18));
        s12 = new QCheckBox(groupBox_3);
        s12->setObjectName(QString::fromUtf8("s12"));
        s12->setGeometry(QRect(170, 130, 31, 18));
        s10 = new QCheckBox(groupBox_3);
        s10->setObjectName(QString::fromUtf8("s10"));
        s10->setGeometry(QRect(90, 130, 31, 18));
        s15 = new QCheckBox(groupBox_3);
        s15->setObjectName(QString::fromUtf8("s15"));
        s15->setGeometry(QRect(290, 130, 31, 18));
        s11 = new QCheckBox(groupBox_3);
        s11->setObjectName(QString::fromUtf8("s11"));
        s11->setGeometry(QRect(130, 130, 31, 18));
        s1 = new QCheckBox(groupBox_3);
        s1->setObjectName(QString::fromUtf8("s1"));
        s1->setGeometry(QRect(50, 110, 31, 18));
        s1->setCheckable(true);
        s1->setChecked(false);
        label_18 = new QLabel(user);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setGeometry(QRect(390, 10, 121, 31));
        QFont font1;
        font1.setPointSize(20);
        font1.setBold(true);
        font1.setWeight(75);
        label_18->setFont(font1);
        comm_bt = new QPushButton(user);
        comm_bt->setObjectName(QString::fromUtf8("comm_bt"));
        comm_bt->setGeometry(QRect(330, 620, 231, 32));
        ETHtest = new QPushButton(user);
        ETHtest->setObjectName(QString::fromUtf8("ETHtest"));
        ETHtest->setGeometry(QRect(640, 630, 75, 23));

        retranslateUi(user);

        QMetaObject::connectSlotsByName(user);
    } // setupUi

    void retranslateUi(QWidget *user)
    {
        user->setWindowTitle(QCoreApplication::translate("user", "\350\212\202\347\202\271\350\256\276\347\275\256\347\225\214\351\235\242", nullptr));
        groupBox->setTitle(QCoreApplication::translate("user", "\346\230\276\347\244\272\345\214\272", nullptr));
        label->setText(QCoreApplication::translate("user", "\346\234\254\345\234\260\347\253\257\345\217\243", nullptr));
        label_2->setText(QCoreApplication::translate("user", "UDP", nullptr));
        UDPOpen->setText(QCoreApplication::translate("user", "\346\211\223\345\274\200UDP", nullptr));
        UDPClose->setText(QCoreApplication::translate("user", "\345\205\263\351\227\255UDP", nullptr));
        ClearAll->setText(QCoreApplication::translate("user", "\346\270\205\347\251\272\347\252\227\345\217\243", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("user", "\345\275\223\345\211\215\347\212\266\346\200\201", nullptr));
        label_3->setText(QCoreApplication::translate("user", "\344\270\273\344\273\216\346\234\272\357\274\232", nullptr));
        MOS_2->setText(QCoreApplication::translate("user", "\344\270\273\346\234\272", nullptr));
        label_9->setText(QCoreApplication::translate("user", "\345\234\260\345\235\200\357\274\232", nullptr));
        label_5->setText(QCoreApplication::translate("user", "STA/AP\357\274\232", nullptr));
        label_6->setText(QCoreApplication::translate("user", "MAC\357\274\232", nullptr));
        label_8->setText(QCoreApplication::translate("user", "SPI\344\270\273\344\273\216\357\274\232", nullptr));
        label_7->setText(QCoreApplication::translate("user", "SPI\351\200\237\347\216\207\357\274\232", nullptr));
        label_10->setText(QCoreApplication::translate("user", "UART\346\263\242\347\211\271\347\216\207\357\274\232", nullptr));
        Address_2->setText(QCoreApplication::translate("user", "7", nullptr));
        MOS_3->setText(QCoreApplication::translate("user", "STA", nullptr));
        MAC->setText(QCoreApplication::translate("user", "\346\234\252\347\237\245", nullptr));
        SPIMOS_2->setText(QCoreApplication::translate("user", "Master", nullptr));
        SPI_rate_2->setText(QCoreApplication::translate("user", "8M", nullptr));
        Baudrate_2->setText(QCoreApplication::translate("user", "115200", nullptr));
        label_25->setText(QCoreApplication::translate("user", "\351\242\221\347\202\271\357\274\232", nullptr));
        Frequency_2->setText(QCoreApplication::translate("user", "1.4M", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("user", "\351\205\215\347\275\256\344\277\241\346\201\257", nullptr));
        label_17->setText(QCoreApplication::translate("user", "\344\270\273\344\273\216\346\234\272\357\274\232", nullptr));
        label_19->setText(QCoreApplication::translate("user", "\345\234\260\345\235\200\357\274\232", nullptr));
        label_20->setText(QCoreApplication::translate("user", "\351\242\221\347\202\271\357\274\232", nullptr));
        label_22->setText(QCoreApplication::translate("user", "SPI\344\270\273\344\273\216\357\274\232", nullptr));
        label_23->setText(QCoreApplication::translate("user", "SPI\351\200\237\347\216\207\357\274\232", nullptr));
        label_24->setText(QCoreApplication::translate("user", "\346\263\242\347\211\271\347\216\207\357\274\232", nullptr));
        ETH_bt->setText(QCoreApplication::translate("user", "\344\270\213\345\217\221\351\205\215\347\275\256", nullptr));
        label_27->setText(QCoreApplication::translate("user", "\350\267\257\347\224\261\357\274\232", nullptr));
        MOS->setItemText(0, QCoreApplication::translate("user", "\344\270\273\346\234\272", nullptr));
        MOS->setItemText(1, QCoreApplication::translate("user", "\344\273\216\346\234\272", nullptr));

        Address->setItemText(0, QCoreApplication::translate("user", "Root(\344\270\273\346\234\272)", nullptr));
        Address->setItemText(1, QCoreApplication::translate("user", "1", nullptr));
        Address->setItemText(2, QCoreApplication::translate("user", "2", nullptr));
        Address->setItemText(3, QCoreApplication::translate("user", "3", nullptr));
        Address->setItemText(4, QCoreApplication::translate("user", "4", nullptr));
        Address->setItemText(5, QCoreApplication::translate("user", "5", nullptr));
        Address->setItemText(6, QCoreApplication::translate("user", "6", nullptr));
        Address->setItemText(7, QCoreApplication::translate("user", "7", nullptr));
        Address->setItemText(8, QCoreApplication::translate("user", "8", nullptr));
        Address->setItemText(9, QCoreApplication::translate("user", "9", nullptr));
        Address->setItemText(10, QCoreApplication::translate("user", "10", nullptr));
        Address->setItemText(11, QCoreApplication::translate("user", "11", nullptr));
        Address->setItemText(12, QCoreApplication::translate("user", "12", nullptr));
        Address->setItemText(13, QCoreApplication::translate("user", "13", nullptr));
        Address->setItemText(14, QCoreApplication::translate("user", "14", nullptr));
        Address->setItemText(15, QCoreApplication::translate("user", "15", nullptr));
        Address->setItemText(16, QCoreApplication::translate("user", "16", nullptr));

        Frequency->setItemText(0, QCoreApplication::translate("user", "1400M", nullptr));
        Frequency->setItemText(1, QCoreApplication::translate("user", "1410M", nullptr));
        Frequency->setItemText(2, QCoreApplication::translate("user", "1420M", nullptr));
        Frequency->setItemText(3, QCoreApplication::translate("user", "1430M", nullptr));
        Frequency->setItemText(4, QCoreApplication::translate("user", "1440M", nullptr));
        Frequency->setItemText(5, QCoreApplication::translate("user", "1450M", nullptr));
        Frequency->setItemText(6, QCoreApplication::translate("user", "1460M", nullptr));
        Frequency->setItemText(7, QCoreApplication::translate("user", "1470M", nullptr));
        Frequency->setItemText(8, QCoreApplication::translate("user", "1480M", nullptr));
        Frequency->setItemText(9, QCoreApplication::translate("user", "1490M", nullptr));
        Frequency->setItemText(10, QCoreApplication::translate("user", "1500M", nullptr));

        SPIMOS->setItemText(0, QCoreApplication::translate("user", "Master", nullptr));
        SPIMOS->setItemText(1, QCoreApplication::translate("user", "Slave", nullptr));

        SPI_rate->setItemText(0, QCoreApplication::translate("user", "8M", nullptr));
        SPI_rate->setItemText(1, QCoreApplication::translate("user", "4M", nullptr));

        Baudrate->setItemText(0, QCoreApplication::translate("user", "115200", nullptr));
        Baudrate->setItemText(1, QCoreApplication::translate("user", "9600", nullptr));

        s2->setText(QCoreApplication::translate("user", "2", nullptr));
        s3->setText(QCoreApplication::translate("user", "3", nullptr));
        s4->setText(QCoreApplication::translate("user", "4", nullptr));
        s5->setText(QCoreApplication::translate("user", "5", nullptr));
        s6->setText(QCoreApplication::translate("user", "6", nullptr));
        s7->setText(QCoreApplication::translate("user", "7", nullptr));
        s8->setText(QCoreApplication::translate("user", "8", nullptr));
        s14->setText(QCoreApplication::translate("user", "14", nullptr));
        s9->setText(QCoreApplication::translate("user", "9", nullptr));
        s13->setText(QCoreApplication::translate("user", "13", nullptr));
        s16->setText(QCoreApplication::translate("user", "16", nullptr));
        s12->setText(QCoreApplication::translate("user", "12", nullptr));
        s10->setText(QCoreApplication::translate("user", "10", nullptr));
        s15->setText(QCoreApplication::translate("user", "15", nullptr));
        s11->setText(QCoreApplication::translate("user", "11", nullptr));
        s1->setText(QCoreApplication::translate("user", "1", nullptr));
        label_18->setText(QCoreApplication::translate("user", "\350\212\202\347\202\271\350\256\276\347\275\256", nullptr));
        comm_bt->setText(QCoreApplication::translate("user", "\347\234\213\346\235\277", nullptr));
        ETHtest->setText(QCoreApplication::translate("user", "ETHtest", nullptr));
    } // retranslateUi

};

namespace Ui {
    class user: public Ui_user {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USER_H
