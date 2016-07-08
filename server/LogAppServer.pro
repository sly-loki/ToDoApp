QT += core network
QT -= gui

TARGET = LogAppServer
CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    core.cpp \
    storage.cpp \
    server.cpp

HEADERS += ../network_def.h \
    core.h \
    storage.h \
    server.h

