#-------------------------------------------------
#
# Project created by QtCreator 2021-01-06T23:43:37
#
#-------------------------------------------------

QT       += core gui network
QT       += webenginewidgets
QT       += webengine
QT       += webchannel
DEFINES += _USE_MATH_DEFINES

INCLUDEPATH+=C:/Users/FPO/Desktop/zhonghang/Upper/WpdPack_4_1_2/WpdPack/Include
LIBS+=C:/Users/FPO/Desktop/zhonghang/Upper/WpdPack_4_1_2/WpdPack/Lib/x64/wpcap.lib
LIBS+=C:/Users/FPO/Desktop/zhonghang/Upper/WpdPack_4_1_2/WpdPack/Lib/x64/Packet.lib
LIBS+=C:/Users/FPO/Desktop/zhonghang/Upper/WpdPack_4_1_2/WpdPack/Lib/libwpcap.a
LIBS+=C:/Users/FPO/Desktop/zhonghang/Upper/WpdPack_4_1_2/WpdPack/Lib/libpacket.a


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Qt_UDP
TEMPLATE = app

SOURCES += main.cpp\
        comm.cpp \
        map.cpp \
        topo.cpp \
        user.cpp \
        welcome.cpp \
        widget.cpp

HEADERS  += widget.h \
    comm.h \
    map.h \
    topo.h \
    user.h \
    welcome.h

FORMS    += widget.ui \
    comm.ui \
    map.ui \
    topo.ui \
    user.ui \
    welcome.ui

RC_ICONS=icons8-cloud.ico

RESOURCES += \
    BAIDUMAP.qrc \
    pic.qrc \
    qt.qrc


#QTQUICK_COMPILER_SKIPPED_RESOURCES += BAIDUMAP.qrc

#UI_DIR   =./UI
#UI_DIR   =/Users/fpo/Desktop/zhonghang/Upper/Qt_UDP/
