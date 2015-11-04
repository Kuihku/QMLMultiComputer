TEMPLATE = app

TARGET = QMLRunner

QT += qml quick widgets network

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

DISTFILES += \
    QML/main.qml

INCLUDEPATH += ../Common
unix: LIBS += -L../Libs -lCommunication
