#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QLabel>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();
    QLabel *progresslabel;
private:
    Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
