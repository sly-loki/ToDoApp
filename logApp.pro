#-------------------------------------------------
#
# Project created by QtCreator 2016-05-30T13:57:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = logApp
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    logtextedit.cpp

HEADERS  += mainwindow.h \
    logtextedit.h

FORMS    += mainwindow.ui
