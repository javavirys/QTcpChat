#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    progresslabel = new QLabel(this);
    progresslabel->setGeometry(QRect(15, 90, 211, 16));
    progresslabel->setAlignment(Qt::AlignCenter);
    progresslabel->setText("");
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}
