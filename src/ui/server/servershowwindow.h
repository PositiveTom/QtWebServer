#ifndef SERVERSHOWWINDOW_H
#define SERVERSHOWWINDOW_H

#include <QMainWindow>
#include <QScrollBar>
#include <QTextEdit>

#include <server/Server.h>

namespace Ui {
class ServerShowWindow;
}

class ServerShowWindow : public QMainWindow
{
    // Q_OBJECT

public:
    explicit ServerShowWindow(Server* srv, QWidget *parent = nullptr);
    ~ServerShowWindow();

private:
    Ui::ServerShowWindow *ui;
    Server* m_srv;
    // QScrollBar* scroll;
};

#endif // SERVERSHOWWINDOW_H
