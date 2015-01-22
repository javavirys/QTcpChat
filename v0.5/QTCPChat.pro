#-------------------------------------------------
#
# Project created by QtCreator 2015-01-21T12:58:00
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTCPChat
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    connecteddialog.cpp

HEADERS  += mainwindow.h \
    connecteddialog.h

FORMS    += mainwindow.ui \
    connecteddialog.ui
