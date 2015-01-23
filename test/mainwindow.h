#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>

#include "connecteddialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    QTcpServer *server;
    QTcpSocket *client;
private:
    Ui::MainWindow *ui;
    ConnectedDialog *conui;
    QList<QTcpSocket *> sockets; // получатели данных
    QTcpSocket *firstSocket; // вещатель
private slots:
    void triggered();
    void stateChanged(QAbstractSocket::SocketState state);
    void Read();
    void ClientConnected();
    void ClientDisconnected();
    void hostFound();
    void proxyAuthenticationRequired(const QNetworkProxy & proxy, QAuthenticator * authenticator);

    void on_SendButton_clicked();

public slots:
    void ConnectionListener();
    void ConnectionType(bool _server);

    void	finished(int result);
};

#endif // MAINWINDOW_H
