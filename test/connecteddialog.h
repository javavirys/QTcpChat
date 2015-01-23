#ifndef CONNECTEDDIALOG_H
#define CONNECTEDDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectedDialog;
}

class ConnectedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectedDialog(QWidget *parent = 0);
    ~ConnectedDialog();
    QString ip;
    int port;
private:
    Ui::ConnectedDialog *ui;
signals:
    void on_accepted(bool server);
    void on_rejected();
private slots:
    void TypeConnectClicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};

#endif // CONNECTEDDIALOG_H
