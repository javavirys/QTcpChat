#include "connecteddialog.h"
#include "ui_connecteddialog.h"

#include <QMessageBox>

ConnectedDialog::ConnectedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectedDialog)
{
    ui->setupUi(this);

    ui->IPEdit->hide();
    ui->label->hide();
    ui->ServerButton->setChecked(true);

    connect(ui->ClientButton,SIGNAL(clicked()),this,SLOT(TypeConnectClicked()));
    connect(ui->ServerButton,SIGNAL(clicked()),this,SLOT(TypeConnectClicked()));
}

ConnectedDialog::~ConnectedDialog()
{
    delete ui;
}

void ConnectedDialog::TypeConnectClicked()
{
    QObject * object = QObject::sender(); // далее и ниже до цикла идет преобразования "отправителя сигнала" в сокет, дабы извлечь данные
    if (!object)
        return;

    QRadioButton * item = static_cast<QRadioButton *>(object);
    if(item==ui->ClientButton)
    {
        ui->IPEdit->show();
        ui->label->show();

    }else if(item==ui->ServerButton)
    {
        ui->IPEdit->hide();
        ui->label->hide();
    }

}


void ConnectedDialog::on_buttonBox_accepted()
{
    bool isServer=false;
    port=ui->PortEdit->text().toInt();
    if(ui->ServerButton->isChecked())
    {
        isServer=true;

    }else
    {
        ip=ui->IPEdit->text();
    }
    emit on_accepted(isServer);
}


void ConnectedDialog::on_buttonBox_rejected()
{
    emit on_rejected();
}
