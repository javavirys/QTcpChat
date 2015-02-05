#-------------------------------------------------
#
# Project created by QtCreator 2015-01-21T12:58:00
#
#-------------------------------------------------

QT       += core gui network
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTCPChat
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    connecteddialog.cpp \
    progressdialog.cpp \
    listeners.cpp

HEADERS  += mainwindow.h \
    connecteddialog.h \
    progressdialog.h \
    listeners.h

FORMS    += mainwindow.ui \
    connecteddialog.ui \
    progressdialog.ui

RESOURCES += \
    res.qrc
