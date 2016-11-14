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

INCLUDEPATH += ../include \
../server

SOURCES += main.cpp\
        mainwindow.cpp \
    logtextedit.cpp \
    guicontrol.cpp \
    core.cpp \
    logappserver.cpp \
    itemwidget.cpp \
    applicationcontrol.cpp \
    settings.cpp \
    ../server/server.cpp \
    ../server/storage.cpp \
    ../server/servercore.cpp \
    settingswindow.cpp

HEADERS  += mainwindow.h \
    logtextedit.h \
    guicontrol.h \
    core.h \
    logappserver.h \
    ../include/network_def.h \
    itemwidget.h \
    applicationcontrol.h \
    settings.h \
    ../server/server.h \
    ../server/storage.h \
    ../include/types.h \
    ../server/servercore.h \
    settingswindow.h

FORMS    += mainwindow.ui \
    settingswindow.ui
