#ifndef SERVERLOGIN_H
#define SERVERLOGIN_H

#include "ui_serverlogin.h"
#include <msg/ServerMsg.h>

#include <QtGui/QIntValidator>
#include <QtWidgets/QDialog>
#include <glog/logging.h>


namespace Ui {
class ServerLogin;
}

class ServerLogin : public QDialog
{
    // Q_OBJECT

public:
    explicit ServerLogin(IpMsg* ipmsg, QWidget *parent = nullptr);
    ~ServerLogin();

private:
    Ui::ServerLogin *ui;
};

#endif // SERVERLOGIN_H
