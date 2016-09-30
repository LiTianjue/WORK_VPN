#-------------------------------------------------
#
# Project created by QtCreator 2016-09-29T10:16:26
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       +=  multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SecurityGatewayServer
TEMPLATE = app


SOURCES +=\
    ../SecurityGateway/down_conf.cpp \
    ../SecurityGateway/handle_json.cpp \
    ../SecurityGateway/handle_xml.cpp \
    ../SecurityGateway/http.cpp \
    ../SecurityGateway/mytcpclient.cpp \
    ../SecurityGateway/public_def.cpp \
    ../SecurityGateway/simple_log.cpp \
    ../SecurityGateway/tconfigfile.cpp \
    ../SecurityGateway/win32_csp.cpp \
    ../SecurityGateway/FaceInfoMan.c \
    ../SecurityGateway/ManagerServer/managerserver.cpp \
    ../SecurityGateway/main.cpp \
    ../SecurityGateway/mainwindow.cpp

HEADERS  += \
    ../SecurityGateway/win32_csp.h \
    ../SecurityGateway/down_conf.h \
    ../SecurityGateway/FaceInfoMan.h \
    ../SecurityGateway/handle_json.h \
    ../SecurityGateway/handle_xml.h \
    ../SecurityGateway/myhelper.h \
    ../SecurityGateway/mytcpclient.h \
    ../SecurityGateway/public_def.h \
    ../SecurityGateway/simple_log.h \
    ../SecurityGateway/tconfigfile.h \
    ../SecurityGateway/ManagerServer/managerserver.h \
    ../SecurityGateway/http.h \
    ../SecurityGateway/mainwindow.h

FORMS    += \
    ../SecurityGateway/mainwindow.ui

INCLUDEPATH +=../SecurityGateway


RESOURCES += \
    ../SecurityGateway/image/resource.qrc

#RC_ICONS = Project.ico

#add for win32 csp api
LIBS +=-L./lib -lcrypt32
#add for win32 socket api
LIBS +=-L./lib -lws2_32

#add for openssl use
#LIBS += -LC:/OpenSSL-Win32/lib/MinGw -leay32 -lssleay32
#INCLUDEPATH += C:/OpenSSL-Win32/include

#goto
QMAKE_CXXFLAGS += -fpermissive
QMAKE_CXXFLAGS += -DMANAGMENT
