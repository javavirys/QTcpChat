#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    firstSocket(NULL)
{
    ui->setupUi(this);

    server=0;
    client=0;
    ui->actionDisconnect->setEnabled(false);
    ui->scrollArea->setVisible(false);
    conui=new ConnectedDialog(this);
    connect(conui,SIGNAL(on_accepted(bool)),this,SLOT(ConnectionType(bool)));
    connect(conui,SIGNAL(finished(int)),this,SLOT(finished(int)));

    connect(ui->actionConnection,SIGNAL(triggered()),this,SLOT(triggered()));
    connect(ui->actionDisconnect,SIGNAL(triggered()),this,SLOT(triggered()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    firstSocket=NULL;
    server=0;
    client=0;
    delete conui;
    delete ui;
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
        if(server)
        {
            server->close();
            delete server;
            server=0;
        }
        if(client)
        {
            client->close();
            delete client;
            client=0;
        }
        ui->TypeLabel->setText("Type: unknown");
        ui->statusLabel->setText("Status: close");
    }
}

void MainWindow::ConnectionListener()
{
    QMessageBox::about(this,"Server","newConn()");
    QTcpSocket *socket=server->nextPendingConnection();
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(stateChanged(QAbstractSocket::SocketState))); // делаем обработчик изменения статуса сокета
    connect(socket, SIGNAL(disconnected()),
            socket, SLOT(deleteLater()));
    if (!firstSocket) { // если у нас нет "вещающего", то данное подключение становится вещающим
            connect(socket, SIGNAL(readyRead()), this, SLOT(Read())); // подключаем входящие сообщения от вещающего на наш обработчик

            ui->scrollArea->setVisible(true);
            ui->statusLabel->setText("Status: connected");

            socket->write("server connected"); // говорим ему что он "вещает"
            firstSocket = socket; // сохраняем себе"
            qDebug() << "this one is server";
        }
        else { // иначе говорим подключенному что он "получатель"
            socket->write("client");
            sockets << socket;
    }
}

void MainWindow::ConnectionType(bool _server)
{
    if(_server)
    {
        server=new QTcpServer(this);
        QObject::connect(server,SIGNAL(newConnection()),this,SLOT(ConnectionListener()));
        //if(server->listen(QHostAddress::Any,conui->port))
        if(server->listen(QHostAddress(conui->ip),conui->port))
        {

            ui->TypeLabel->setText("Type: Server");
            ui->statusLabel->setText("Status: listen");
        }else
        {
            QMessageBox::about(this,"Server","!Listen");
        }

    }else
    {
        //QMessageBox::about(this,"Client","Not realized()");
        client=new QTcpSocket(this);
        connect(client, SIGNAL(readyRead()), this, SLOT(Read())); // подключаем входящие сообщения от вещающего на наш обработчик
        connect(client,SIGNAL(disconnected()),this,SLOT(ClientDisconnected()));
        connect(client,SIGNAL(connected()),this,SLOT(ClientConnected()));
        connect(client,SIGNAL(hostFound()),this,SLOT(hostFound()));
        client->connectToHost(QHostAddress(conui->ip),conui->port);
        if(client->waitForConnected(10000))
        {
            ui->TypeLabel->setText("Type: Client");
            ui->statusLabel->setText("Status: connected");
        }
    }
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
    case 1:

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
            ui->scrollArea->setVisible(false);

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
    ui->scrollArea->setVisible(true);
    firstSocket=client;
}

void MainWindow::ClientDisconnected()
{
    ui->statusLabel->setText("Status: disconnect");
    ui->scrollArea->setVisible(false);
    firstSocket=NULL;
}

void MainWindow::hostFound()
{

}

void MainWindow::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
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
