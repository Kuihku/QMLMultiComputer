TEMPLATE = app

TARGET = QMLRunner

QT = core gui quick qml widgets quickwidgets

CONFIG += c++11

SOURCES += \
    main.cpp \
    runnerview.cpp

HEADERS += \
    runnerview.h

DISTFILES += \
    QML/main.qml

INCLUDEPATH += ../Common
unix: LIBS += -L../Libs -lCommunication
