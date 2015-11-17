QT       = core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ApplicationServer
TEMPLATE = app


SOURCES += \
    main.cpp \
    server.cpp \
    localconnection.cpp \
    remoteconnection.cpp \
    mainview.cpp \
    remoteapplication.cpp

HEADERS += \
    server.h \
    localconnection.h \
    remoteconnection.h \
    mainview.h \
    windowholder.h \
    remoteconnectioninfo.h \
    remoteapplication.h

# TARGET_PLATFORM = arm

equals(TARGET_PLATFORM, "arm") {
    DEFINES += SERVER_CONFIG=\\\"/home/tuheimon/QMLMultiComputer/ApplicationServer/server_config.txt\\\"
    DEFINES += APPLICATION_PATH=\\\"/home/tuheimon/QMLMultiComputer/ApplicationServer/Applications\\\"
    DEFINES += QMLRUNNEREXE=\\\"/home/tuheimon/QMLMultiComputer/QMLRunner/QMLRunner\\\"
} else: {
    DEFINES += SERVER_CONFIG=\\\"$$PWD/server_config.txt\\\"
    DEFINES += APPLICATION_PATH=\\\"$$PWD/Applications\\\"
    DEFINES += MYIPADDRESS=\\\"192.168.1.200\\\"
    DEFINES += QMLRUNNEREXE=\\\"$$PWD/../QMLRunner/QMLRunner\\\"
}

DISTFILES += \
    server_config.txt

INCLUDEPATH += ../Common
unix: LIBS += -L../Libs -lCommunication
