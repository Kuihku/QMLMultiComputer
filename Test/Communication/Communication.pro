#-------------------------------------------------
#
# Project created by QtCreator 2015-11-10T08:38:34
#
#-------------------------------------------------

QT       = core gui network testlib

TARGET = tst_communicationtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_communicationtest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
DEFINES += COMMUNICATION_TEST=true

LIBS += -L$$OUT_PWD/../../Libs -lCommunication

INCLUDEPATH += $$PWD/../../Common
DEPENDPATH += $$PWD/../../Communication
