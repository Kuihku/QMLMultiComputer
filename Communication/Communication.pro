QT       += network
QT       -= gui

TARGET = Communication
TEMPLATE = lib
DESTDIR = $$PWD\..\Libs
DEFINES += COMMUNICATION_LIBRARY

SOURCES += \
    message.cpp

HEADERS +=\
    message.h

unix {
    QMAKE_POST_LINK += cp -f message.h ../Common/message.h
    QMAKE_CLEAN += ../Common/message.h
}

win32 {
    QMAKE_POST_LINK += copy /Y message.h ..\\Common\\message.h
    QMAKE_CLEAN += ..\\Common\\message.h
}

# LIBS += -L../Libs -lCommunication
