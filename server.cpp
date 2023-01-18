#include "server.h"

void Server::_SendMessageToClients(QString message, QString name, QString address, QString avatarka, QString ava_width, QString ava_height)
{
    QJsonObject jsonobj;
    jsonobj.insert("message", message);
    jsonobj.insert("date", QDateTime::currentDateTime().toString());
    jsonobj.insert("username", name);
    jsonobj.insert("address", address);
    jsonobj.insert("avatarka", avatarka);
    jsonobj.insert("avatarka_width", ava_width);
    jsonobj.insert("avatarka_height", ava_height);
    _chat_messages.append(jsonobj);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    for (auto i: _clients_map)
    {
        i.first->sendBinaryMessage(arrJson);
    }
}

void Server::_SendImageToClients(QString image, QString im_width, QString im_height, QString name, QString address, QString avatarka, QString ava_width, QString ava_height)
{
    QJsonObject jsonobj;
    jsonobj.insert("image", image);
    jsonobj.insert("image_width", im_width);
    jsonobj.insert("image_height", im_height);
    jsonobj.insert("date", QDateTime::currentDateTime().toString());
    jsonobj.insert("username", name);
    jsonobj.insert("address", address);
    jsonobj.insert("avatarka", avatarka);
    jsonobj.insert("avatarka_width", ava_width);
    jsonobj.insert("avatarka_height", ava_height);
    _chat_messages.append(jsonobj);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    for (auto i: _clients_map)
    {
        i.first->sendBinaryMessage(arrJson);
    }
}

void Server::_SendFileToClients(QString file, QString file_name, QString name, QString address, QString avatarka, QString ava_width, QString ava_height)
{
    QJsonObject jsonobj;
    jsonobj.insert("file", file);
    jsonobj.insert("file_name", file_name);
    jsonobj.insert("date", QDateTime::currentDateTime().toString());
    jsonobj.insert("username", name);
    jsonobj.insert("address", address);
    jsonobj.insert("avatarka", avatarka);
    jsonobj.insert("avatarka_width", ava_width);
    jsonobj.insert("avatarka_height", ava_height);
    _chat_messages.append(jsonobj);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    for (auto i: _clients_map)
    {
        i.first->sendBinaryMessage(arrJson);
    }
}

void Server::_SendHistoryToClient(QWebSocket* server_socket)
{
    QJsonDocument json;
    json.setArray(_chat_messages);
    QByteArray arrJson(json.toJson());
    server_socket->sendBinaryMessage(arrJson);
}

void Server::_SendInfoToClients(QString name, QString ip, QString status, QString time, QString avatarka, QString ava_width, QString ava_height, bool is_new)
{
    QJsonObject jsonobj;
    jsonobj.insert("ip", ip);
    jsonobj.insert("time", time);
    jsonobj.insert("username", name);
    jsonobj.insert("status", status);
    jsonobj.insert("avatarka", avatarka);
    jsonobj.insert("avatarka_width", ava_width);
    jsonobj.insert("avatarka_height", ava_height);
    jsonobj.insert("is_new", is_new);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    for (auto i: _clients_map)
    {
        i.first->sendBinaryMessage(arrJson);
    }
}

void Server::reconnectClients()
{
    QJsonObject jsonobj;
    jsonobj.insert("reconnect", true);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    for (auto i: _clients_map)
    {
        i.first->sendBinaryMessage(arrJson);
    }
}

void Server::changeIpAndPort()
{
    QDialog* modalDialog = new QDialog(_textbrowser);
    modalDialog->setModal(true);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    QLabel *Label = new QLabel("Server");
    Label->setAlignment(Qt::AlignCenter);
    boxLayout1->addWidget(Label);

    QPushButton *exit_button = new QPushButton("Close");
    connect(exit_button, SIGNAL(clicked()), modalDialog, SLOT(close()));
    QPushButton *save_button = new QPushButton("Save");
    connect(save_button, SIGNAL(clicked()), this, SLOT(saveIpAndPort()));

    QBoxLayout* boxLayout2 = new QBoxLayout(QBoxLayout::LeftToRight);
    QLabel *ipLabel = new QLabel("IP: ");
    boxLayout2->addWidget(ipLabel);
    boxLayout2->addWidget(new_ip);

    QBoxLayout* boxLayout3 = new QBoxLayout(QBoxLayout::LeftToRight);
    QLabel *portLabel = new QLabel("PORT: ");
    boxLayout3->addWidget(portLabel);
    boxLayout3->addWidget(new_port);

    boxLayout1->addLayout(boxLayout2);
    boxLayout1->addLayout(boxLayout3);
    boxLayout1->addWidget(save_button);
    boxLayout1->addWidget(exit_button);

    modalDialog->setLayout(boxLayout1);
    modalDialog->exec();
}

void Server::saveIpAndPort()
{
    try
    {
        if (new_port->text() != "")
        {
            auto temp = new_port->text().toInt();
            if(temp >= 0)
            {
                _port = new_port->text();
                new_port->setText(_port);
            }
            else
            {
                qDebug() << "Error in setting new port!";
            }
        }
        if (new_ip->text() != "")
        {
            _ip = new_ip->text();
            new_ip->setText(_ip);
        }
        _SendDisconnectToClients();
        _clients_map.clear();
        this->close();
        if (this->listen(QHostAddress(_ip), _port.toUShort()))
        {
            _mainwindow->setWindowTitle("Hosting Server on " + _ip + ":" + _port + ", clients: 0");
            qDebug() << "Socket started";
        }
        else
        {
            qDebug() << "Socket failed to start";
        }
    }
    catch(...)
    {
        QMessageBox::warning(this->_mainwindow, "Ошибка!", "Введён неверный порт или порт!");
    }
}

void Server::_SendDisconnectToClients()
{
    QJsonObject jsonobj;
    jsonobj.insert("disconnect", true);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    for (auto i: _clients_map)
    {
        i.first->sendBinaryMessage(arrJson);;
    }
}

void Server::_SendInfoToOneClient(QWebSocket* server_socket, QString name, QString ip, QString status, QString time, QString avatarka, QString ava_width, QString ava_height, bool is_new)
{
    QJsonObject jsonobj;
    jsonobj.insert("ip", ip);
    jsonobj.insert("time", time);
    jsonobj.insert("username", name);
    jsonobj.insert("status", status);
    jsonobj.insert("avatarka", avatarka);
    jsonobj.insert("avatarka_width", ava_width);
    jsonobj.insert("avatarka_height", ava_height);
    jsonobj.insert("is_new", is_new);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    server_socket->sendBinaryMessage(arrJson);
}

void Server::_SendPongToClient(QWebSocket* server_socket)
{
    QJsonObject jsonobj;
    jsonobj.insert("connected", true);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    server_socket->sendBinaryMessage(arrJson);
}

void Server::onExit()
{
    _SendDisconnectToClients();
}

void Server::saveToXML()
{
    QDomDocument doc;
    QDomElement passw = doc.createElement("password");
    QDomText passwText = doc.createTextNode(_password.toUtf8().toBase64(QByteArray::Base64Encoding));
    QDomElement gigablock = doc.createElement("gigablock");
    doc.appendChild(passw);
    passw.appendChild(passwText);
    doc.appendChild(gigablock);
    for(auto i: _chat_messages)
    {
        QDomElement block = doc.createElement("block");
        QDomElement date = doc.createElement("date");
        QDomText dateText = doc.createTextNode(i.toObject()["date"].toString());
        QDomElement address = doc.createElement("address");
        QDomText addressText = doc.createTextNode(i.toObject()["address"].toString());
        QDomElement message;
        QString text;
        if (i.toObject().contains("message"))
        {
            message = doc.createElement("message");
            text = i.toObject()["message"].toString();
            text = QString::fromUtf8(text.toUtf8().toBase64(QByteArray::Base64Encoding));
            QDomText messageText = doc.createTextNode(text);
            message.appendChild(messageText);
        }
        else if (i.toObject().contains("image"))
        {
            message = doc.createElement("image");
            text = i.toObject()["image"].toString();
            QByteArray temp(QByteArray::fromBase64(text.toUtf8()));
            text = QString::fromUtf8(qCompress(temp).toBase64(QByteArray::Base64Encoding));
            QDomText messageText = doc.createTextNode(text);
            message.appendChild(messageText);
        }
        else if (i.toObject().contains("file"))
        {
            message = doc.createElement("file");
            QDomElement file_id = doc.createElement("file_id");
            QString file_temp = i.toObject()["file"].toString();
            QDomElement file_name = doc.createElement("file_name");
            QString file_text = i.toObject()["file_name"].toString();
            QDomText file_name_text = doc.createTextNode(file_text);
            file_name.appendChild(file_name_text);
            QByteArray temp(QByteArray::fromBase64(file_temp.toUtf8()));
            QDomElement file_hash = doc.createElement("file_hash");
            QCryptographicHash hashing(QCryptographicHash::Algorithm::Md5);
            hashing.addData(temp);
            QString file_temp_hash = QString::fromUtf8(hashing.result().toBase64());
            QDomText file_hash_text = doc.createTextNode(file_temp_hash);
            file_hash.appendChild(file_hash_text);
            message.appendChild(file_name);
            message.appendChild(file_hash);
            std::string curpath = QDir::currentPath().toStdString();
            QUuid uuid;
            uuid = QUuid::createUuidV5(uuid, i.toObject()["file_name"].toString());
            QString id = uuid.toString(QUuid::StringFormat::Id128);
            QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1)) + id + ".txt";
            QFile file2(path);
            file2.open(QIODeviceBase::WriteOnly);
            file2.write(temp);
            file2.close();
            QDomText file_id_text = doc.createTextNode(path);
            file_id.appendChild(file_id_text);
            message.appendChild(file_id);
        }
        QDomElement name = doc.createElement("name");
        QDomText nameText = doc.createTextNode(i.toObject()["username"].toString());
        date.appendChild(dateText);
        address.appendChild(addressText);
        name.appendChild(nameText);
        block.appendChild(date);
        block.appendChild(address);
        block.appendChild(name);
        block.appendChild(message);
        gigablock.appendChild(block);
    }
    new_password->setText("");
    _password = "";
    file.setFileName(_path);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    doc.save(out, 4);
    _path = "";
}

void Server::MenuTriggered_on()
{
    if (this->listen(QHostAddress(_ip), _port.toUShort()))
    {
        _mainwindow->setWindowTitle("Hosting Server on " + _ip + ":" + _port + ", clients: 0");
        qDebug() << "Socket started";
    }
    else
    {
        qDebug() << "Socket failed to start";
    }
}

void Server::MenuTriggered_off()
{
    _SendDisconnectToClients();
    this->close();
    _mainwindow->setWindowTitle("Server is OFF on " + _ip + ":" + _port + ", clients: 0");
}

void Server::on_actionDialogClosedTriggered(int i)
{
    if (file.isOpen()) file.close();
}

void Server::on_actionHideShowPasswordTriggered(int checked)//скрытие и показ пароля
{
    if (checked == 2)
    {
        new_password->setEchoMode(QLineEdit::Password);
    }
    else
    {
        new_password->setEchoMode(QLineEdit::Normal);
    }
}

void Server::MenuTriggered_save()
{
    QDialog* modalDialog = new QDialog(_mainwindow);
    modalDialog->setModal(true);
    connect(modalDialog, SIGNAL(finished(int)), this, SLOT(on_actionDialogClosedTriggered(int)));

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    QLabel* Label = new QLabel("XML");
    Label->setAlignment(Qt::AlignCenter);
    boxLayout1->addWidget(Label);

    QPushButton* save_button = new QPushButton("Write to .xml");
    connect(save_button, &QPushButton::clicked, this, &Server::saveToXML);
    connect(save_button, SIGNAL(clicked()), modalDialog, SLOT(close()));

    QPushButton* path_button = new QPushButton("Choose path");
    connect(path_button, &QPushButton::clicked, this, &Server::choosePathTriggered);

    QCheckBox* hide_password_checkbox = new QCheckBox("Hide?");
    connect(hide_password_checkbox, SIGNAL(stateChanged(int)), this, SLOT(on_actionHideShowPasswordTriggered(int)));

    QBoxLayout* boxLayout2 = new QBoxLayout(QBoxLayout::LeftToRight);
    QLabel* labelPassword = new QLabel("Password: ");
    boxLayout2->addWidget(labelPassword);
    boxLayout2->addWidget(new_password);
    boxLayout2->addWidget(hide_password_checkbox);

    QBoxLayout* boxLayout3 = new QBoxLayout(QBoxLayout::LeftToRight);
    boxLayout3->addWidget(_file_name);
    boxLayout3->addWidget(path_button);

    boxLayout1->addLayout(boxLayout2);
    boxLayout1->addLayout(boxLayout3);
    boxLayout1->addWidget(save_button);

    modalDialog->setLayout(boxLayout1);
    modalDialog->exec();
}

void Server::choosePathTriggered()
{
    _path = QFileDialog::getSaveFileName(_mainwindow, tr("Save"), "C:/Users", "*.xml");
    _file_name->setText(_path);
    _password = new_password->text();
}

void Server::MenuTriggered_exit()
{
    this->close();
    _mainwindow->close();
    _mainwindow->deleteLater();
}

void Server::slotDisconnected()
{
    QWebSocket* server_socket = (QWebSocket*) sender();
    QString temptext = QDateTime::currentDateTime().toString() + "\tUser " +
            server_socket->peerAddress().toString() + ":" +
            QString::number(server_socket->peerPort()) + " has DISCONNECTED";
    QString redtext = QString("<span style=\" color:#ff0000;\">%1</span>").arg(temptext);
    _textbrowser->append(redtext);
    for (auto i = _clients_map.begin(); i != _clients_map.end(); i++)
    {
        if (i->first->peerAddress() == server_socket->peerAddress() && i->first->peerPort() == server_socket->peerPort())
        {
            _SendInfoToClients(_clients_map[server_socket]->name, server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort()), _clients_map[server_socket]->status, _clients_map[server_socket]->time, _clients_map[server_socket]->avatarka, (_clients_map[server_socket])->ava_width, (_clients_map[server_socket])->ava_height, false);
            delete i->second;
            _clients_map.erase(i);
            break;
        }
    }
    QHostAddress hostaddress = QHostAddress::LocalHost;
    _mainwindow->setWindowTitle("Hosting Server on " + hostaddress.toString() + ":" + "45678" + ", clients: " + QString::number(_clients_map.size()));
}

void Server::incomingConnectionAuth(QWebSocketCorsAuthenticator* authenticator)
{
    authenticator->setAllowed(true);
}

void Server::incomingConnection()
{
    QWebSocket* server_socket = nextPendingConnection();
    connect(server_socket, SIGNAL(binaryMessageReceived(const QByteArray&)), this, SLOT(slotReadyRead(const QByteArray&)));
    connect(server_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    QString temptext = QDateTime::currentDateTime().toString() + "\tUser " +
                server_socket->peerAddress().toString() + ":" +
                QString::number(server_socket->peerPort()) + " has CONNECTED";
    QString greentext = QString("<span style=\" color:#228B22;\">%1</span>").arg(temptext);
    _textbrowser->append(greentext);
    _clients_map[server_socket] = new Clients("", "", "", "", "", "");
    _SendInfoToClients((_clients_map[server_socket])->name, server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort()), (_clients_map[server_socket])->status, (_clients_map[server_socket])->time, (_clients_map[server_socket])->avatarka, (_clients_map[server_socket])->ava_width, (_clients_map[server_socket])->ava_height, true);
    _mainwindow->setWindowTitle("Hosting Server on " + _ip + ":" + _port + ", clients: " + QString::number(_clients_map.size()));
    for (auto i = _clients_map.begin(); i != _clients_map.end(); i++)
    {
        if (i->first->peerAddress() != server_socket->peerAddress() || i->first->peerPort() != server_socket->peerPort())
        {
            _SendInfoToOneClient(server_socket, i->second->name, i->first->peerAddress().toString() + ":" + QString::number(i->first->peerPort()), i->second->status, i->second->time, i->second->avatarka, i->second->ava_width, i->second->ava_height, true);
        }
        if(i->first->peerAddress() == server_socket->peerAddress() && i->first->peerPort() == server_socket->peerPort())
        {
            _SendPongToClient(server_socket);
        }
    }
}

void Server::slotReadyRead(const QByteArray& arr)
{
            QWebSocket* server_socket = (QWebSocket*) sender();
            qDebug()<<"json";
            QJsonDocument json = QJsonDocument::fromJson(arr);
            QJsonObject jsonobj = json.object();
            if (jsonobj.contains("history"))//если пришёл запрос на получение истории
            {
                _SendHistoryToClient(server_socket);
            }
            else if (jsonobj.contains("new_status") && jsonobj.contains("new_username") && jsonobj.contains("new_avatarka"))//если у пользователя новый статус или никнейм или ава
            {
                qDebug()<<"new client";
                QString name = jsonobj["new_username"].toString();
                QString status = jsonobj["new_status"].toString();
                QString avatarka = jsonobj["new_avatarka"].toString();
                QString ava_width = jsonobj["avatarka_width"].toString();
                QString ava_height = jsonobj["avatarka_height"].toString();
                _SendInfoToClients((_clients_map[server_socket])->name, server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort()), (_clients_map[server_socket])->status, (_clients_map[server_socket])->time, (_clients_map[server_socket])->avatarka, (_clients_map[server_socket])->ava_width, (_clients_map[server_socket])->ava_height, false);
                for (auto i = _clients_map.begin(); i != _clients_map.end(); i++)
                {
                    if (i->first->peerAddress() == server_socket->peerAddress() && i->first->peerPort() == server_socket->peerPort())
                    {
                        (i->second)->name = name;
                        (i->second)->status = status;
                        (i->second)->avatarka = avatarka;
                        (i->second)->ava_width = ava_width;
                        (i->second)->ava_height = ava_height;
                        if ((i->second)->time == "")
                        {
                            (i->second)->time = QDateTime::currentDateTime().toString();
                        }
                        break;
                    }
                }
                qDebug()<<"New: " + server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort());
                _SendInfoToClients((_clients_map[server_socket])->name, server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort()), (_clients_map[server_socket])->status, (_clients_map[server_socket])->time, (_clients_map[server_socket])->avatarka, (_clients_map[server_socket])->ava_width, (_clients_map[server_socket])->ava_height, true);
            }
            else if (jsonobj.contains("message"))//просто пришло сообщение
            {
                qDebug()<<"message";
                QString message = jsonobj["message"].toString();
                QString name = jsonobj["username"].toString();
                QString status = jsonobj["status"].toString();
                QString avatarka = jsonobj["avatarka"].toString();
                QString ava_width = jsonobj["avatarka_width"].toString();
                QString ava_height = jsonobj["avatarka_height"].toString();
                _SendMessageToClients(message, name, server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort()), avatarka, ava_width, ava_height);
            }
            else if (jsonobj.contains("image"))//если пришла картинка
            {
                qDebug()<<"image";
                QString image = jsonobj["image"].toString();
                QString im_width = jsonobj["image_width"].toString();
                QString im_height = jsonobj["image_height"].toString();
                QString name = jsonobj["username"].toString();
                QString status = jsonobj["status"].toString();
                QString avatarka = jsonobj["avatarka"].toString();
                QString ava_width = jsonobj["avatarka_width"].toString();
                QString ava_height = jsonobj["avatarka_height"].toString();
                _SendImageToClients(image, im_width, im_height, name, server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort()), avatarka, ava_width, ava_height);
            }
            else if (jsonobj.contains("file"))//если пришёл файл
            {
                qDebug()<<"file";
                QString file = jsonobj["file"].toString();
                QString file_name = jsonobj["file_name"].toString();
                QString name = jsonobj["username"].toString();
                QString status = jsonobj["status"].toString();
                QString avatarka = jsonobj["avatarka"].toString();
                QString ava_width = jsonobj["avatarka_width"].toString();
                QString ava_height = jsonobj["avatarka_height"].toString();
                _SendFileToClients(file, file_name, name, server_socket->peerAddress().toString() + ":" + QString::number(server_socket->peerPort()), avatarka, ava_width, ava_height);
            }
}




