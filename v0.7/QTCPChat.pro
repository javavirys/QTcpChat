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
    listeners.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    connecteddialog.h \
    progressdialog.h \
    listeners.h \
    aboutdialog.h

FORMS    += mainwindow.ui \
    connecteddialog.ui \
    progressdialog.ui \
    aboutdialog.ui

RESOURCES += \
    res.qrc
win32:RC_FILE = icon.rc
