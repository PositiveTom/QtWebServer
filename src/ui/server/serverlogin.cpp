#include "serverlogin.h"

ServerLogin::ServerLogin(IpMsg* ipmsg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerLogin)
{
    ui->setupUi(this);

    ui->lineEdit->setInputMask("000.000.000.000;");

    QIntValidator *intVal = new QIntValidator();
    ui->lineEdit_2->setValidator(intVal);

    this->setFixedSize(271, 212);

    this->connect(ui->pushButton, &QPushButton::clicked, this, [=](){
        QString ip = ui->lineEdit->text();
        QString port = ui->lineEdit_2->text();
        // LOG(INFO) << ip.toStdString() ;
        // LOG(INFO) << port.toInt() ;
        ipmsg->ip = ip.toStdString();
        ipmsg->port = port.toInt();

        this->done(0);
    });
}

ServerLogin::~ServerLogin()
{
    delete ui;
}
