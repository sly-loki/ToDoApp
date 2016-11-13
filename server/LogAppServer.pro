QT += core network
QT -= gui

TARGET = LogAppServer
CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += main.cpp \
    servercore.cpp \
    storage.cpp \
    server.cpp

HEADERS += ../include/network_def.h \
    servercore.h \
    storage.h \
    server.h \
    ../include/types.h

