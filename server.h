#ifndef SERVER_H
#define SERVER_H

#include <QWebSocketServer>
#include <QWebSocket>
#include <QVector>
#include <QTextBrowser>
#include <algorithm>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "mainwindow.h"
#include <map>
#include <algorithm>
#include <QMessageBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QBoxLayout>
#include <QBuffer>
#include <QByteArray>
#include <QWebSocketCorsAuthenticator>
#include <QMenu>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFileDialog>
#include <QCheckBox>
#include <QSettings>
#include <fstream>
#include <QCryptographicHash>
#include <QDialog>
#include <QUuid>
#include <QDataStream>

struct Clients
{
    QString name;
    QString status;
    QString time;
    QString avatarka;
    QString ava_width;
    QString ava_height;
    Clients(QString name, QString status, QString time, QString avatarka, QString ava_width, QString ava_height)
    {
        this->name = name;
        this->status = status;
        this->time = time;
        this->avatarka = avatarka;
        this->ava_width = ava_width;
        this->ava_height = ava_height;
    }
};

class Server : public QWebSocketServer
{
    Q_OBJECT
public:
    Server(MainWindow* mainwindow, QTextBrowser* textbrowser, QHBoxLayout* box, const QString &serverName, QWebSocketServer::SslMode secureMode): QWebSocketServer(serverName, secureMode)
    {
        this->_mainwindow = mainwindow;
        this->_textbrowser = textbrowser;
        this->_box = box;
        new_ip = new QLineEdit;
        new_port = new QLineEdit;
        new_password = new QLineEdit;
        QHostAddress hostaddress = QHostAddress::LocalHost;
        new_ip->setText(hostaddress.toString());
        _ip = hostaddress.toString();
        new_port->setText("45678");
        _port = "45678";
        QPushButton* change_address = new QPushButton("Change address");
        connect(change_address, &QPushButton::clicked, this, &Server::changeIpAndPort);
        QPushButton* change_key = new QPushButton("Change key");
        connect(change_key, &QPushButton::clicked, this, &Server::reconnectClients);
        _box->addWidget(change_address);
        _box->addWidget(change_key);
        _mainwindow->setWindowTitle("Server is OFF on " + _ip + ":" + _port + ", clients: 0");
        connect(_mainwindow, &MainWindow::destroyed, this, &Server::onExit);
        connect(this, SIGNAL(originAuthenticationRequired(QWebSocketCorsAuthenticator *)), this, SLOT(incomingConnectionAuth(QWebSocketCorsAuthenticator *)));
        connect(this, SIGNAL(newConnection()), this, SLOT(incomingConnection()));
        QPushButton* button_file = new QPushButton("File");
        button_file->setText("File");
        menu = new QMenu("File");
        QAction* action_on= new QAction();
        action_on->setData(0);
        action_on->setText("Turn on");
        connect(action_on, SIGNAL(triggered()), this, SLOT(MenuTriggered_on()));
        menu->addAction(action_on);
        QAction* action_off= new QAction();
        action_off->setData(1);
        action_off->setText("Turn off");
        connect(action_off, SIGNAL(triggered()), this, SLOT(MenuTriggered_off()));
        menu->addAction(action_off);
        QAction* action_save= new QAction();
        action_save->setData(2);
        action_save->setText("Save to XML");
        connect(action_save, SIGNAL(triggered()), this, SLOT(MenuTriggered_save()));
        menu->addAction(action_save);
        QAction* action_exit= new QAction();
        action_exit->setData(3);
        action_exit->setText("Exit");
        connect(action_exit, SIGNAL(triggered()), this, SLOT(MenuTriggered_exit()));
        menu->addAction(action_exit);
        button_file->setMenu(menu);
        _box->addWidget(change_key);
        _box->addWidget(button_file);
        _password = "";
        _file_name = new QLabel();
    }

private:
    std::map<QWebSocket*, Clients*> _clients_map;// (имя, статус, время подсоединения)
    QJsonArray _chat_messages;
    QString _port;
    QString _ip;
    void _SendInfoToClients(QString name, QString ip, QString status, QString time, QString avatarka, QString ava_width, QString ava_height, bool is_new);
    void _SendInfoToOneClient(QWebSocket* server_socket, QString name, QString ip, QString status, QString time, QString avatarka, QString ava_width, QString ava_height, bool is_new);
    void _SendMessageToClients(QString message, QString name, QString ip, QString avatarka, QString ava_width, QString ava_height);
    void _SendImageToClients(QString image, QString im_width, QString im_height, QString name, QString address, QString avatarka, QString ava_width, QString ava_height);
    void _SendFileToClients(QString file, QString file_name, QString name, QString address, QString avatarka, QString ava_width, QString ava_height);
    void _SendHistoryToClient(QWebSocket* server_socket);
    void _SendDisconnectToClients();
    void _SendPongToClient(QWebSocket* server_socket);
    MainWindow* _mainwindow;
    QTextBrowser* _textbrowser;
    QHBoxLayout* _box;
    QLineEdit* new_ip;
    QLineEdit* new_port;
    QString _password;
    QLineEdit* new_password;
    QString _path;
    QFile file;
    QMenu* menu;
    QLabel* _file_name;

private slots:
    void incomingConnection();
    void incomingConnectionAuth(QWebSocketCorsAuthenticator* authenticator);
    void slotReadyRead(const QByteArray& arr);
    void slotDisconnected();
    void reconnectClients();
    void changeIpAndPort();
    void saveIpAndPort();
    void saveToXML();
    void MenuTriggered_on();
    void MenuTriggered_off();
    void MenuTriggered_save();
    void MenuTriggered_exit();
    void choosePathTriggered();
    void on_actionDialogClosedTriggered(int i);
    void on_actionHideShowPasswordTriggered(int checked);
    void onExit();
};

#endif // SERVER_H
