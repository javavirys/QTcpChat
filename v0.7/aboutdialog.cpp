#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "mainwindow.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_commandLinkButton_clicked()
{
    QApplication::aboutQt();
}
