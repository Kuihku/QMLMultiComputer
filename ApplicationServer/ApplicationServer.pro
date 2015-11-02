QT       = core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QMLPainter
TEMPLATE = app


SOURCES += \
    main.cpp \
    mainview.cpp

HEADERS += \
    mainview.h

