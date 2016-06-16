#-------------------------------------------------
#
# Project created by QtCreator 2016-05-30T13:57:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = logApp
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    logtextedit.cpp \
    guicontrol.cpp \
    storage.cpp \
    core.cpp \
    logappserver.cpp

HEADERS  += mainwindow.h \
    logtextedit.h \
    guicontrol.h \
    storage.h \
    core.h \
    logappserver.h \
    network_def.h

FORMS    += mainwindow.ui
