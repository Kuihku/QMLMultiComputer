QT       = core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ApplicationServer
TEMPLATE = app


SOURCES += \
    main.cpp \
    server.cpp \
    localconnection.cpp \
    remoteconnection.cpp

HEADERS += \
    server.h \
    localconnection.h \
    remoteconnection.h


DEFINES += SERVER_CONFIG=\\\"$$PWD/server_config.txt\\\"

DISTFILES += \
    server_config.txt

INCLUDEPATH += ../Common
unix: LIBS += -L../Libs -lCommunication
