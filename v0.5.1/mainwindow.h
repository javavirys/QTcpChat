#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>

#include <QNetworkReply>

#include "connecteddialog.h"
#include "progressdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    void RegisterOnServer(const QString &_url);
    void InitServer(int _port);
    void InitClient(const QString &_ip,int _port);
    QTcpServer *server;
    QTcpSocket *client;
private:
    Ui::MainWindow  *ui;
    ConnectedDialog *conui;
    ProgressDialog  *progress;
    QList<QTcpSocket *> sockets; // получатели данных
    QTcpSocket *firstSocket; // вещатель
    QNetworkAccessManager *manager;
    bool downloaderror;
    bool isserver;
    bool isconnectionclosed;

    QString Host;
    QString Password;
    QString IP;
    int Port;

private slots:
    void triggered();
    void stateChanged(QAbstractSocket::SocketState state);
    void Read();
    void ClientConnected();
    void ClientDisconnected();
    void hostFound();

    void on_SendButton_clicked();

    void on_DownloadError(QNetworkReply::NetworkError code);
    void on_DownloadFinished(QNetworkReply * reply);
public slots:
    void ConnectionListener();
    void ConnectionType(bool _server);

    void	finished(int result);
};

#endif // MAINWINDOW_H
