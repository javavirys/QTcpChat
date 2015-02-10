#include "listeners.h"
#include <QMessageBox>

Listeners::Listeners(QObject *parent) :
    QObject(parent),
    wnd(0)
{
    wnd=static_cast<MainWindow*>(parent);
}

void Listeners::triggered()
{
    QObject * object = QObject::sender();
    if (!object)
        return;
    QRect rect;

    QAction * action = static_cast<QAction *>(object);
    if(action==wnd->actionConnection)
    {
        rect=wnd->geometry();
        wnd->conui->setGeometry(rect.x()+(rect.width()/2)-(wnd->conui->width()/2),
                                rect.y()+(rect.height()/2)-(wnd->conui->height()/2),
                                wnd->conui->width(),wnd->conui->height());
        wnd->conui->show();
    }else if(action==wnd->actionDisconnect)
    {
        wnd->actionConnection->setEnabled(true);
        wnd->actionDisconnect->setEnabled(false);

        wnd->SendButton->setEnabled(false);

        if(wnd->server)
        {
            wnd->server->close();
            delete wnd->server;
            wnd->server=NULL;

            wnd->isconnectionclosed=true;
            wnd->progress->setModal(true);
            wnd->progress->show();
            QString temp;
            temp="http://coder.pusku.com/projects/QTCPChat/server.php?delHost=1&&host=";
            temp+=wnd->Host+"&&pass="+wnd->Password;
            temp+=QString("&port=%1").arg(wnd->Port);
            wnd->progress->progresslabel->setText("Disconnecting...");
            wnd->RegisterOnServer(temp);
        }
        if(wnd->client)
        {
            wnd->client->close();
            delete wnd->client;
            wnd->client=NULL;
            wnd->TypeLabel->setText("Type: unknown");
            wnd->statusLabel->setText("Status: close");
        }

    }else if(action==wnd->actionRestore)
    {
        wnd->showNormal();
        wnd->tray->hide();
    }else if(action==wnd->actionAbout)
    {
        rect=wnd->geometry();
        wnd->about->setGeometry(rect.x()+(rect.width()/2)-(wnd->about->width()/2),
                                rect.y()+(rect.height()/2)-(wnd->about->height()/2),
                                249,324);
        wnd->about->show();
    }
}

void Listeners::clicked()
{
    QObject * object = QObject::sender();
    if (!object)
        return;

    QPushButton * button = static_cast<QPushButton *>(object);
    if(button==wnd->SendButton)
    {

        wnd->workEdit->setTextColor(QColor(40,200,40));
        QString text=QDateTime::currentDateTime().toString("hh:mm:ss");
        text+=" ";
        text+=wnd->messageEdit->text();
        wnd->workEdit->append(text);
        if (wnd->firstSocket) {
            wnd->firstSocket->write(wnd->messageEdit->text().toLocal8Bit());
        }

    }
}

void Listeners::changeEvent(QEvent *event)
{
    if(event->type()==QEvent::WindowStateChange)
    {
        if(wnd->isMinimized()==true)
        {
            wnd->tray->show();
            wnd->hide();
        }
    }
}
