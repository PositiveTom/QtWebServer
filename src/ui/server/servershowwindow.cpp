#include "servershowwindow.h"
#include "ui_servershowwindow.h"

ServerShowWindow::ServerShowWindow(Server* srv, QWidget *parent) : m_srv(srv),
    QMainWindow(parent),
    ui(new Ui::ServerShowWindow)
{
    ui->setupUi(this);
    srv->AddObserver(ui->textEdit); /*添加观察者*/

    /*QTextEdit自带滑动条*/
    // ui->verticalScrollBar->setRange(0, 100);
    // connect(ui->verticalScrollBar, &QScrollBar::valueChanged, ui->textEdit->verticalScrollBar(), &QScrollBar::setValue);

    ui->textEdit->setReadOnly(true);
    // ui->textEdit->append("hello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\nhello\n");

    ui->statusbar->showMessage(("HttpServer: IP Adreess = " + srv->GetIp() + ",  Port = " + std::to_string(srv->GetPort())).c_str());
}

ServerShowWindow::~ServerShowWindow()
{
    delete ui;
}
