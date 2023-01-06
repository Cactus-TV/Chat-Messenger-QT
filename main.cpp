#include "mainwindow.h"
#include "server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow* w = new MainWindow;
    Server server(w, w->ReturnTextBrowser(), w->ReturnBox(), "Laboratornaya", QWebSocketServer::SslMode::NonSecureMode);
    w->show();
    return a.exec();
}
