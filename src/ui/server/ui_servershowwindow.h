/********************************************************************************
** Form generated from reading UI file 'servershowwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERSHOWWINDOW_H
#define UI_SERVERSHOWWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

// QT_BEGIN_NAMESPACE

class Ui_ServerShowWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_2;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QTextEdit *textEdit;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *ServerShowWindow)
    {
        if (ServerShowWindow->objectName().isEmpty())
            ServerShowWindow->setObjectName(QString::fromUtf8("ServerShowWindow"));
        ServerShowWindow->resize(800, 600);
        centralwidget = new QWidget(ServerShowWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout_2 = new QHBoxLayout(centralwidget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        widget = new QWidget(centralwidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        textEdit = new QTextEdit(widget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));

        horizontalLayout->addWidget(textEdit);


        horizontalLayout_2->addWidget(widget);

        ServerShowWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(ServerShowWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        ServerShowWindow->setStatusBar(statusbar);

        retranslateUi(ServerShowWindow);

        QMetaObject::connectSlotsByName(ServerShowWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ServerShowWindow)
    {
        ServerShowWindow->setWindowTitle(QApplication::translate("ServerShowWindow", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ServerShowWindow: public Ui_ServerShowWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERSHOWWINDOW_H
