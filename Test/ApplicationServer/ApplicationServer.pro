#-------------------------------------------------
#
# Project created by QtCreator 2015-11-07T11:57:29
#
#-------------------------------------------------

QT       += widgets network testlib

TARGET = tst_applicationservertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../ApplicationServer ../../Common

SOURCES += tst_applicationservertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += TESTTCPSERVERIPADDRESS=\\\"192.168.1.201\\\"
DEFINES += SERVEREXE=\\\"$$PWD/../../ApplicationServer/ApplicationServer\\\"
DEFINES += SERVER_CONFIG=\\\"$$PWD/server_config.txt\\\"

DISTFILES += \
    server_config.txt

LIBS += -L../../Libs -lCommunication
