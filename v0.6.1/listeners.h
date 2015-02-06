#ifndef LISTENERS_H
#define LISTENERS_H

#include <QObject>
#include <QAction>
#include "mainwindow.h"

class Listeners : public QObject
{
    Q_OBJECT
public:
    explicit Listeners(QObject *parent = 0);
protected:
    class MainWindow *wnd;
signals:

public slots:
    void triggered();
    void clicked();
    void changeEvent(QEvent *event);
};

#endif // LISTENERS_H
