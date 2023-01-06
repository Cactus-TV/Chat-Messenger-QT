#ifndef CLIENT_H
#define CLIENT_H

#include <QtCore>
#include <QTcpSocket>
#include "mainwindow.h"

class Client : public QMainWindow
{
    Q_OBJECT
public:
    static Client& Instance()
    {
        static Client client;
        return client;
    }
    void init(QObject* parent = 0);
    void connectToServer(QString address, quint16 port);
    void sendToServer(QString message);

signals:
    void newMessage(QString str);

private:
    Client() {}
    ~Client() {}
    Client(const Client&);
    Client& operator=(const Client&);

    QTcpSocket * clientSocket;
    QByteArray transportingData;
private slots:
    void slotReadyToRead();
};

#endif // CLIENT_H
