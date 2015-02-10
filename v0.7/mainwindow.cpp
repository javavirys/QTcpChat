#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDateTime>

#include <QSound>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    firstSocket=NULL;
    downloaderror=false;
    isserver=false;
    isconnectionclosed=false;
    server=NULL;
    client=NULL;
    ui->actionDisconnect->setEnabled(false);

    about=new AboutDialog(this);
    about->setWindowFlags(Qt::Tool);
    about->setModal(true);

    conui=new ConnectedDialog(this);
    conui->setWindowFlags(Qt::Tool);
    conui->setModal(true);
    connect(conui,SIGNAL(on_accepted(bool)),this,SLOT(ConnectionType(bool)));

    progress=new ProgressDialog(this);
    progress->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    progress->setModal(true);

    TypeLabel=new QLabel("Type: unknown",ui->centralWidget);
    TypeLabel->setGeometry(QRect(10, 0, 251, 16));
    statusLabel=new QLabel("Status: Disconnect",ui->centralWidget);
    statusLabel->setGeometry(QRect(10, 20, 251, 16));
    //иниц.
    workEdit = new QTextEdit(ui->scrollAreaWidgetContents);
    workEdit->setGeometry(QRect(10, 10, 231, 261));
    workEdit->setStyleSheet(QStringLiteral(""));
    workEdit->setReadOnly(true);

    listener=new Listeners(this);
    messageEdit = new QLineEdit(ui->scrollAreaWidgetContents);
    messageEdit->setGeometry(QRect(10, 280, 161, 21));

    SendButton = new QPushButton(ui->scrollAreaWidgetContents);
    connect(SendButton,SIGNAL(clicked()),listener,SLOT(clicked()));
    SendButton->setText("Send");
    SendButton->setGeometry(QRect(180, 280, 61, 21));
    SendButton->setEnabled(false);

    actionRestore=new QAction(QIcon(":/images/mainwindow/Resources/revert.png"),"Restore",this);
    actionQuit=new QAction(QIcon(":/images/mainwindow/Resources/quit.png"),"Quit",this);
    actionConnection=new QAction(QIcon(":/images/mainwindow/Resources/connect.png"),"Connect",this);
    actionDisconnect=new QAction(QIcon(":/images/mainwindow/Resources/disconnect.png"),"Disconnect",this);
    actionDisconnect->setEnabled(false);
    actionAbout=new QAction(QIcon(":/images/mainwindow/Resources/about.png"),"About",this);

    menuHelp=new QMenu("Help",this);
    menuHelp->addAction(actionAbout);
    ui->menuBar->addMenu(menuHelp);

    //Создание главного меню
    ui->menuQTCPChat->addAction(actionConnection);
    ui->menuQTCPChat->addAction(actionDisconnect);
    ui->menuQTCPChat->addSeparator();
    ui->menuQTCPChat->addAction(actionQuit);
    connect(actionConnection,SIGNAL(triggered()),listener,SLOT(triggered()));
    connect(actionDisconnect,SIGNAL(triggered()),listener,SLOT(triggered()));
    connect(actionRestore,SIGNAL(triggered()),listener,SLOT(triggered()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(actionAbout,SIGNAL(triggered()),listener,SLOT(triggered()));

    manager = new QNetworkAccessManager(this);
    disconnect(manager, SIGNAL(finished(QNetworkReply*)),
            0, 0);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(on_DownloadFinished(QNetworkReply*)));

    tray=new QSystemTrayIcon(QIcon(":/images/tray/Resources/offline.png"));
    trayMenu=new QMenu(this);
    trayMenu->addAction(actionRestore);
    trayMenu->addSeparator();
    trayMenu->addAction(actionQuit);
    tray->setContextMenu(trayMenu);
}

MainWindow::~MainWindow()
{
    isserver=false;
    firstSocket=NULL;
    server=NULL;
    client=NULL;

    delete tray;
    delete trayMenu;

    delete messageEdit;

    delete SendButton;

    delete workEdit;

    delete TypeLabel;
    delete statusLabel;

    delete menuHelp;

    delete actionQuit;
    delete actionRestore;
    delete actionDisconnect;
    delete actionConnection;
    delete actionAbout;

    delete listener;
    delete manager;
    manager=NULL;

    delete about;
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
}

void MainWindow::InitServer(int _port)
{
    server=new QTcpServer(this);
    QObject::connect(server,SIGNAL(newConnection()),this,SLOT(ConnectionListener()));
    if(server->listen(QHostAddress::Any,_port))
    {
        TypeLabel->setText("Type: Server");
        statusLabel->setText("Status: listen");
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
        TypeLabel->setText("Type: Client");
    }else
    {
        QMessageBox::about(this,"Client","Timeout");
    }
    progress->hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(client || server)
    {
        QMessageBox::about(this,"Warning","You not disconnected!!!");
        event->ignore();
    }else
    {
        event->accept();
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
            statusLabel->setText("Status: connected");
            SendButton->setEnabled(true);
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

    tray->setIcon(QIcon(":/images/tray/Resources/online.png"));

    actionConnection->setEnabled(false);
    actionDisconnect->setEnabled(true);

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
    progress->show();
    progress->progresslabel->setText("Connecting...");
    RegisterOnServer(temp);
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
            statusLabel->setText("Status: listen");
            SendButton->setEnabled(false);

        }
}

void MainWindow::Read()
{
    QObject * object = QObject::sender(); // далее и ниже до цикла идет преобразования "отправителя сигнала" в сокет, дабы извлечь данные
        if (!object)
            return;

        QTcpSocket * socket = static_cast<QTcpSocket *>(object);
        QByteArray arr =  socket->readAll();

        workEdit->setTextColor(QColor(200,40,40));
        QDateTime curTime=QDateTime::currentDateTime();
        QString text=curTime.toString("hh:mm:ss");
        text+=" ";
        text+=arr;
        workEdit->append(text);
        QSound::play(":/sounds/Resources/newmessage.wav");//requist qt5.2.2
        if(tray->isVisible())
        {
            tray->showMessage("New Message!!!",text,QSystemTrayIcon::Information);
        }
}

void MainWindow::ClientConnected()
{
    statusLabel->setText("Status: connected");
    SendButton->setEnabled(true);
    firstSocket=client;
}

void MainWindow::ClientDisconnected()
{
    statusLabel->setText("Status: disconnect");
    SendButton->setEnabled(false);
    firstSocket=NULL;
}

void MainWindow::hostFound()
{
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
        TypeLabel->setText("Type: unknown");
        statusLabel->setText("Status: close");
        isconnectionclosed=false;
        tray->setIcon(QIcon(":/images/tray/Resources/offline.png"));
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
            if(progress->isVisible()){
                progress->progresslabel->setText("Init client");
            }
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
    workEdit->clear();
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type()==QEvent::WindowStateChange)
    {
        if(isMinimized()==true)
        {
            tray->setToolTip("QTcpChat");
            tray->show();
            this->hide();
        }
    }
}
