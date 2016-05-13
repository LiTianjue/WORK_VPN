#-------------------------------------------------
#
# Project created by QtCreator 2015-03-03T17:17:03
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       +=  multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SecurityGateway
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tconfigfile.cpp \
    http.cpp \
    win32_csp.cpp \
    down_conf.cpp \
    handle_xml.cpp \
    mytcpclient.cpp \
    FaceInfoMan.c \
    public_def.cpp \
    simple_log.cpp \
    handle_json.cpp

HEADERS  += mainwindow.h \
    tconfigfile.h \
    public_def.h \
    http.h \
    win32_csp.h \
    down_conf.h \
    FaceInfoMan.h \
    handle_xml.h \
    myhelper.h \
    mytcpclient.h \
    simple_log.h \
    handle_json.h

FORMS    += mainwindow.ui

RESOURCES += \
    image/resource.qrc

RC_ICONS = Project.ico

#add for win32 csp api
LIBS +=-L./lib -lcrypt32
#add for win32 socket api
LIBS +=-L./lib -lws2_32

#add for openssl use
#LIBS += -LC:/OpenSSL-Win32/lib/MinGw -leay32 -lssleay32
#INCLUDEPATH += C:/OpenSSL-Win32/include

# goto
QMAKE_CXXFLAGS += -fpermissive
