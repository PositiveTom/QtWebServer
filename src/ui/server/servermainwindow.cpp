#include "servermainwindow.h"
#include "ui_servermainwindow.h"

ServerMainWindow::ServerMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerMainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(721, 445);
}

ServerMainWindow::~ServerMainWindow()
{
    delete ui;
}
