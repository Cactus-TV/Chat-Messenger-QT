#include "client.h"

void Client::init(QObject * parent)
{
    clientSocket = new QTcpSocket(parent);
    connect(clientSocket, &QTcpSocket::readyRead, this, &Client::slotReadyToRead);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
}

void Client::connectToServer(QString address, quint16 port)
{
    clientSocket->connectToHost(address, port);
}

void Client::sendToServer(QString message)
{
    transportingData.clear();
    QDataStream out(&transportingData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_3);
    out << message;
    clientSocket->write(transportingData);
}

void Client::slotReadyToRead()
{
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_6_3);
    if (in.status() == QDataStream::Ok) {
        QString str;
        in >> str;
        emit newMessage(str);
    }
}
