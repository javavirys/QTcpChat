#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDateTime>

#include <QDialog>

#include <QNetworkRequest>

#include <QtXml>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    firstSocket(NULL)
{
    ui->setupUi(this);
    downloaderror=false;
    isserver=false;
    isconnectionclosed=false;
    server=NULL;
    client=NULL;
    ui->actionDisconnect->setEnabled(false);

    ui->SendButton->setEnabled(false);

    conui=new ConnectedDialog(this);
    connect(conui,SIGNAL(on_accepted(bool)),this,SLOT(ConnectionType(bool)));
    connect(conui,SIGNAL(finished(int)),this,SLOT(finished(int)));

    progress=new ProgressDialog(this);
    progress->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);

    connect(ui->actionConnection,SIGNAL(triggered()),this,SLOT(triggered()));
    connect(ui->actionDisconnect,SIGNAL(triggered()),this,SLOT(triggered()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(on_DownloadFinished(QNetworkReply*)));
}

MainWindow::~MainWindow()
{
    isserver=false;
    firstSocket=NULL;
    server=NULL;
    client=NULL;

    delete manager;
    manager=NULL;

    delete progress;
    delete conui;
    delete ui;
}

void MainWindow::RegisterOnServer(const QString &_url)
{

    QNetworkRequest request;
    request.setUrl(QUrl(_url));
    request.setRawHeader("User-Agent", "QTcpChat 1.0");

    QNetworkReply *reply =manager->get(request);

    disconnect(reply,SIGNAL(error(QNetworkReply::NetworkError)) , 0, 0);
    //connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(on_DownloadError(QNetworkReply::NetworkError)));
    //connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
    //        this, SLOT(slotSslErrors(QList<QSslError>)));
}

void MainWindow::InitServer(int _port)
{
    server=new QTcpServer(this);
    QObject::connect(server,SIGNAL(newConnection()),this,SLOT(ConnectionListener()));
    if(server->listen(QHostAddress::Any,_port))
    {
        ui->TypeLabel->setText("Type: Server");
        ui->statusLabel->setText("Status: listen");
    }else
    {
        QMessageBox::about(this,"Server","Error listener");
    }
    progress->hide();
}

void MainWindow::InitClient(const QString &_ip, int _port)
{
    client=new QTcpSocket(this);
    connect(client, SIGNAL(readyRead()), this, SLOT(Read())); // подключаем входящие сообщения от вещающего на наш обработчик
    connect(client,SIGNAL(disconnected()),this,SLOT(ClientDisconnected()));
    connect(client,SIGNAL(connected()),this,SLOT(ClientConnected()));
    connect(client,SIGNAL(hostFound()),this,SLOT(hostFound()));
    client->connectToHost(QHostAddress(_ip),_port);
    if(client->waitForConnected())
    {
        ui->TypeLabel->setText("Type: Client");
    }else
    {
        QMessageBox::about(this,"Client","Timeout");
    }
    progress->hide();
}

void MainWindow::triggered()
{
    QObject * object = QObject::sender(); // далее и ниже до цикла идет преобразования "отправителя сигнала" в сокет, дабы извлечь данные
    if (!object)
        return;

    QAction * menuitem = static_cast<QAction *>(object);
    if(menuitem==ui->actionConnection)
    {
        ui->actionConnection->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        conui->setModal(true);
        conui->show();
    }else if(menuitem==ui->actionDisconnect)
    {
        ui->actionConnection->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        ui->SendButton->setEnabled(false);
        if(server)
        {
            server->close();
            delete server;
            server=NULL;

            isconnectionclosed=true;
            progress->setModal(true);
            progress->show();
            QString temp;
            temp="http://coder.pusku.com/projects/QTCPChat/server.php?delHost=1&&host=";
            temp+=Host+"&&pass="+Password;
            temp+=QString("&port=%1").arg(Port);
            RegisterOnServer(temp);
        }
        if(client)
        {
            client->close();
            delete client;
            client=NULL;
            ui->TypeLabel->setText("Type: unknown");
            ui->statusLabel->setText("Status: close");
        }

    }
}

void MainWindow::ConnectionListener()
{
    QTcpSocket *socket=server->nextPendingConnection();
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(stateChanged(QAbstractSocket::SocketState))); // делаем обработчик изменения статуса сокета
    connect(socket, SIGNAL(disconnected()),
            socket, SLOT(deleteLater()));
    if (!firstSocket) { // если у нас нет "вещающего", то данное подключение становится вещающим
            connect(socket, SIGNAL(readyRead()), this, SLOT(Read())); // подключаем входящие сообщения от вещающего на наш обработчик

            ui->statusLabel->setText("Status: connected");
            ui->SendButton->setEnabled(true);
            socket->write("server connected"); // говорим ему что он "вещает"
            firstSocket = socket; // сохраняем себя
   }

}

void MainWindow::ConnectionType(bool _server)
{
    isserver=_server;
    QString temp;
    Host=conui->host;
    Password=conui->password;

    if(isserver)
    {
        Port=conui->port;
        temp="http://coder.pusku.com/projects/QTCPChat/server.php?host=";
        temp+=Host+"&&pass="+Password;
        temp+=QString("&port=%1").arg(Port);
    }else
    {
        temp="http://coder.pusku.com/projects/QTCPChat/search.php?host=";
        temp+=Host+"&&pass="+Password;
    }

    progress->setModal(true);
    progress->show();
    RegisterOnServer(temp);
}

void MainWindow::finished(int result)
{
    switch (result) {
    case 0:
        if(server || client)
        {
            ui->actionConnection->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);

        }else
        {
            ui->actionConnection->setEnabled(true);
            ui->actionDisconnect->setEnabled(false);

        }
        break;
    default:
        break;
    }
}

void MainWindow::stateChanged(QAbstractSocket::SocketState state)
{
    QObject * object = QObject::sender();
        if (!object)
            return;
        QTcpSocket * socket = static_cast<QTcpSocket *>(object);
        qDebug() << state;
        if (socket == firstSocket && state == QAbstractSocket::UnconnectedState)
        {
            firstSocket = NULL;
            ui->statusLabel->setText("Status: listen");
            ui->SendButton->setEnabled(false);

        }
}

void MainWindow::Read()
{
    QObject * object = QObject::sender(); // далее и ниже до цикла идет преобразования "отправителя сигнала" в сокет, дабы извлечь данные
        if (!object)
            return;

        QTcpSocket * socket = static_cast<QTcpSocket *>(object);
        QByteArray arr =  socket->readAll();

        ui->workEdit->setTextColor(QColor(200,40,40));
        QDateTime curTime=QDateTime::currentDateTime();
        QString text=curTime.toString("hh:mm:ss");
        text+=" ";
        text+=arr;
        ui->workEdit->append(text);


}

void MainWindow::ClientConnected()
{
    ui->statusLabel->setText("Status: connected");
    ui->SendButton->setEnabled(true);
    firstSocket=client;
}

void MainWindow::ClientDisconnected()
{
    ui->statusLabel->setText("Status: disconnect");
    ui->SendButton->setEnabled(false);
    firstSocket=NULL;
}

void MainWindow::hostFound()
{
}


void MainWindow::on_SendButton_clicked()
{
    ui->workEdit->setTextColor(QColor(40,200,40));
    QString text=QDateTime::currentDateTime().toString("hh:mm:ss");
    text+=" ";
    text+=ui->messageEdit->text();
    ui->workEdit->append(text);
    if (firstSocket) {
        firstSocket->write(ui->messageEdit->text().toLocal8Bit());
    }
}

void MainWindow::on_DownloadError(QNetworkReply::NetworkError code)
{
    progress->hide();
    QMessageBox::about(this,"status",QString("Error: %1").arg(code));
    downloaderror=true;
}

int getTextByTag(QString *dst,const QString &text,const QString &tag)
{
    int spos,epos;
    QString stag,etag;
    spos=epos=0;
    stag="<"+tag+">";
    etag="</"+tag+">";
    if((spos=text.indexOf(stag))==-1 || (epos=text.indexOf(etag))==-1)
        return -1;
    dst->clear();
    dst->append(text.mid(spos+stag.length(),epos-(spos+stag.length())));
    return 1;
}

void MainWindow::on_DownloadFinished(QNetworkReply *reply)
{
    if(downloaderror)
    {
        downloaderror=false;
        return;
    }
    if(isconnectionclosed)
    {
        Host.clear();
        Password.clear();
        IP.clear();
        Port=0;
        progress->hide();
        ui->TypeLabel->setText("Type: unknown");
        ui->statusLabel->setText("Status: close");
        isconnectionclosed=false;
        return;
    }
    QByteArray array=reply->readAll();
    QString text;

    if(getTextByTag(&text,QString(array),"status")==-1)
    {
        QMessageBox::about(this,"Error","Tag not found");
        return;
    }
    switch(text.toInt())
    {
    case 200:
        if(isserver)
        {
            InitServer(Port);
        }
        else
        {
            if(getTextByTag(&text,QString(array),"ip")==-1)
            {
                QMessageBox::about(this,"Error","Tag \'ip\' not found");
                return;
            }
            IP=text;
            if(getTextByTag(&text,QString(array),"port")==-1)
            {
                QMessageBox::about(this,"Error","Tag \'port' not found");
                return;
            }
            Port=text.toInt();
            InitClient(IP,Port);
        }
        break;
    default:
        progress->hide();
        QMessageBox::about(this,"Error!!!","Error on server");
    }
    ui->workEdit->clear();
}
