#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    _socket = new QWebSocket("", QWebSocketProtocol::VersionLatest, this);
    _socket->ignoreSslErrors();
    connect(_socket, SIGNAL(binaryMessageReceived(const QByteArray&)), this, SLOT(slotReadyRead(const QByteArray&)));
    connect(this, &MainWindow::destroyed, this, &MainWindow::on_actionExit_triggered);
    connect(_ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(on_actionInfoAboutUserTriggered(QListWidgetItem*)));
    connect(_ui->listWidget_2, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(OpenImageFile(QListWidgetItem*)));

    std::string curpath = QDir::currentPath().toStdString();
    QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1) + "Chat_Client/settings.ini");
    settings = new QSettings(path, QSettings::IniFormat);
    UploadSettings();
    _ui->action->setChecked(true);
    _ui->action_2->setChecked(false);
    _ui->action_3->setChecked(false);
    new_ip = new QLineEdit;
    new_ip->setText(_ip);
    new_port = new QLineEdit;
    new_port->setText(_port);
    this->setWindowTitle("Status: " + _user_status);
    _ui->actionServer->setText(_ip+":"+_port);
    new_password = new QLineEdit;
    new_username = new QLineEdit;
    new_username->setText(_username);
    _file_name = new QLabel("");
    _path = "";
    new_status = new QLineEdit;
    button_send = new QExPushButton("Send text");
    button_send->setText("Send text");
    connect(button_send, SIGNAL(clicked_left()), this, SLOT(Show_MultiButton_clicked()));
    connect(button_send, SIGNAL(clicked_right()), this, SLOT(Send_Button_clicked()));
    _chosen_image = new QLabel;
    photo = new QLabel;

    #ifdef CRYPTO
    this->_sok->setPeerVerifyMode(QSslSocket::VerifyNone);
    connect(_sok, &QSslSocket::encrypted, this, &MainWindow::onConnected);
    #endif // CRYPTO

    menu = new QMenu("Send text");

    action_image= new QAction();
    action_image->setData(0);
    action_image->setText("Image");
    connect(action_image, SIGNAL(triggered()), this, SLOT(MenuTriggered2()));

    action_file= new QAction();
    action_file->setData(2);
    action_file->setText("File");
    connect(action_file, SIGNAL(triggered()), this, SLOT(MenuTriggered3()));

    action_default= new QAction();
    action_default->setData(1);
    action_default->setText("Text");
    connect(action_default, SIGNAL(triggered()), this, SLOT(MenuTriggered1()));

    menu->addAction(action_default);
    menu->addAction(action_image);
    menu->addAction(action_file);
    button_send->setMenu(menu);
    _ui->splitter->addWidget(button_send);
}

void MainWindow::MenuTriggered1()//мультикнопка по умолчанию
{
    button_send->setText("Send text");
    menu->setTitle("Send text");
    menu->setVisible(false);
}

void MainWindow::MenuTriggered2()//мультикнопка изображение
{
    button_send->setText("Send image");
    menu->setTitle("Send image");
    menu->setVisible(false);
}

void MainWindow::MenuTriggered3()//мультикнопка файл
{
    button_send->setText("Send file");
    menu->setTitle("Send file");
    menu->setVisible(false);
}

MainWindow::~MainWindow()
{
    for (auto& i: _users_vector)
    {
        delete i;
    }
    _users_vector.clear();
    _images_map.clear();
    delete _ui;
}

void MainWindow::on_actionHideShowPasswordTriggered(int checked)//скрытие и показ пароля
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

void MainWindow::OpenImageFile(QListWidgetItem *item)
{
    if(_images_map.contains(item))
    {
        QMenu* cursorMenu = new QMenu();
        QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::Down);
        QPushButton* button_open = new QPushButton("Open original image");
        QPushButton* button_save = new QPushButton("Save image");
        connect(button_open, SIGNAL(clicked()), this, SLOT(FullSizeImage()));
        connect(button_save, SIGNAL(clicked()), this, SLOT(SaveImage()));
        connect(button_open, SIGNAL(clicked()), cursorMenu, SLOT(close()));
        connect(button_save, SIGNAL(clicked()), cursorMenu, SLOT(close()));
        boxLayout->addWidget(button_open);
        boxLayout->addWidget(button_save);
        cursorMenu->setLayout(boxLayout);
        _chosen_image = new QLabel();
        QString imstr = std::get<0>(_images_map[item]);
        int w = std::get<1>(_images_map[item]);
        int h = std::get<2>(_images_map[item]);
        QByteArray array = QByteArray::fromBase64(imstr.toUtf8());
        QImage im((uchar *)array.data(), w, h, QImage::Format_RGB32);
        QPixmap image = QPixmap::fromImage(im);
        QPixmap temp=image.scaledToWidth(this->width()/2, Qt::SmoothTransformation);
        temp=temp.scaledToHeight(this->height()/2, Qt::SmoothTransformation);
        _chosen_image->setPixmap(temp);
        cursorMenu->popup(QCursor::pos());
    }
    else if(_files_map.contains(item))
    {
        QMenu* cursorMenu = new QMenu();
        QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::Down);
        QPushButton* button_open = new QPushButton("Open file");
        QPushButton* button_save = new QPushButton("Save file");
        connect(button_open, SIGNAL(clicked()), this, SLOT(OpenFile()));
        connect(button_save, SIGNAL(clicked()), this, SLOT(SaveFile()));
        connect(button_open, SIGNAL(clicked()), cursorMenu, SLOT(close()));
        connect(button_save, SIGNAL(clicked()), cursorMenu, SLOT(close()));
        boxLayout->addWidget(button_open);
        boxLayout->addWidget(button_save);
        cursorMenu->setLayout(boxLayout);
        _chosen_file = _files_map[item];
        cursorMenu->popup(QCursor::pos());
    }
}

void MainWindow::Change_Show_Time()
{
    QString name = "";
    QString date = "";
    QString message = "";
    for(int i = 0; i < _ui->listWidget_2->count(); ++i)
    {
        auto temp = _ui->listWidget_2->item(i);
        name = std::get<0>(_messages_map[temp]);
        date = std::get<1>(_messages_map[temp]);
        message = std::get<2>(_messages_map[temp]);
        if(_show_time)
        {
            temp->setText(date + " (" + name + "): " + message);
        }
        else
        {
            temp->setText("(" + name + "): " + message);
        }
    }

}

void MainWindow::OpenFile()
{
    std::string curpath = QDir::currentPath().toStdString();
    QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1) + "Chat_Client");
    QDir dir_temp(path);//создаём temp директорию
    dir_temp.mkdir("temp");
    QFile file(path + "/temp/" + std::get<1>(_chosen_file));
    if (!file.exists())
    {
        file.open(QIODevice::WriteOnly);
        QString t1 = std::get<0>(_chosen_file);
        QByteArray t2 = QByteArray::fromBase64(t1.toUtf8());
        file.write(t2);
        file.close();
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path + "/temp/" + std::get<1>(_chosen_file)));
}

void MainWindow::SaveFile()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Save"), "C:/Users");
    QFile file2(path + "/" + std::get<1>(_chosen_file));
    file2.open(QIODevice::WriteOnly);
    QString t1 = std::get<0>(_chosen_file);
    QByteArray t2 = QByteArray::fromBase64(t1.toUtf8());
    file2.write(t2);
    file2.close();
}

void MainWindow::FullSizeImage()
{
    QDialog* modalDialog = new QDialog(this);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    boxLayout1->addWidget(_chosen_image);

    QPushButton *exit_button = new QPushButton("Close");
    connect(exit_button, SIGNAL(clicked()), modalDialog, SLOT(close()));
    boxLayout1->addWidget(exit_button);

    modalDialog->setLayout(boxLayout1);
    modalDialog->exec();
}

void MainWindow::SaveImage()
{
    QString filters("PNG (*.png);;XPM (*.xpm);;JPG (*.jpg*)");
    QString defaultFilter("PNG (*.png)");
    QString path = QFileDialog::getSaveFileName(this, tr("Save"), "C:/Users", filters, &defaultFilter);
    QImageWriter writer(path);
    QPixmap image = _chosen_image->pixmap();
    writer.write(image.toImage());
}

void MainWindow::on_actionOpenPhotoTriggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open"), "C:/Users", tr("Images (*.png *.xpm *.jpg)"));
    QImageReader* img = new QImageReader(path);
    _photo = QPixmap::fromImageReader(img);
    if(!_photo.isNull())
    {
        QPixmap image = _photo.scaledToWidth(this->size().width()/2, Qt::SmoothTransformation);
        image = image.scaledToHeight(this->size().height()/2, Qt::SmoothTransformation);
        photo->setPixmap(image);
        if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
        {
            SendNewUsernameOrStatusOrAvaToServer();
        }
    }
}

void MainWindow::slotReadyRead(const QByteArray &arr)
{
            QJsonDocument json = QJsonDocument::fromJson(arr);
            if (json.isNull() || json.isEmpty())
            {
                return;
            }
            if (json.isArray())//история сообщений
            {
                _history = json.array();
                if (!_path.isEmpty() && !_password.isEmpty())
                {
                    SaveToXML();
                }
                else
                {
                    QMessageBox::warning(this, "Внимание!", "Выберите путь для сохранения файла,\nа также введите пароль!");
                }
            }
            else if (json.isObject())//новый пользователь или сообщение или переподключение
            {
                qDebug() << "json";
                QJsonObject jsonobj = json.object();
                if (jsonobj.contains("connected"))//сообщение о том, что сервер авторизовал подключение
                {
                    SendNewUsernameOrStatusOrAvaToServer();
                }
                else if (jsonobj.contains("reconnect"))//команда сменить ключ и переподключиться
                {
                    on_actionDisconnect_triggered();

                    #ifdef CRYPTO
                    generate_key();
                    #endif // CRYPTO

                    on_actionConnect_triggered();
                }
                else if (jsonobj.contains("disconnect"))
                {
                    on_actionDisconnect_triggered();
                }
                else if (jsonobj.contains("file"))
                {
                    qDebug() << "file";
                    QString file = jsonobj["file"].toString();
                    QString file_name = jsonobj["file_name"].toString();
                    QString name = jsonobj["username"].toString();//имя
                    QString date = jsonobj["date"].toString();//дата и время сообщения
                    QString address = jsonobj["address"].toString();//ip и порт
                    QString avatarka = jsonobj["avatarka"].toString();//аватарка
                    int ava_width = jsonobj["avatarka_width"].toString().toInt();
                    int ava_height = jsonobj["avatarka_height"].toString().toInt();
                    QListWidgetItem *item1 = new QListWidgetItem;
                    if (address == _socket->peerAddress().toString() && name == _username)//если это ваше сообщение
                    { 
                        name = "YOU";
                    }
                    if (_user_status != "NoDisturb")//проигрывать звук, если не стоит режим не беспокоить
                    {
                        std::string curpath = QDir::currentPath().toStdString();
                        QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1) + "Chat_Client/sms.wav");
                        QMediaPlayer effect;
                        effect.setSource(QUrl::fromLocalFile(path));
                        effect.play();
                        QApplication::beep();
                    }
                    QString str = "";
                    if(_show_time)
                    {
                        str = date + " (" + name + "): ";
                    }
                    else
                    {
                        str = "(" + name + "): ";
                    }
                    _messages_map[item1] = std::make_tuple(name, date, file_name);
                    _files_map[item1] = std::make_tuple(file, file_name);
                    if (!avatarka.isEmpty())
                    {
                        QByteArray array2 = QByteArray::fromBase64(avatarka.toUtf8());
                        QImage im2((uchar *)array2.data(), ava_width, ava_height, QImage::Format_RGB32);
                        QPixmap im22 = QPixmap::fromImage(im2);
                        im22=im22.scaledToWidth(200, Qt::SmoothTransformation);
                        im22=im22.scaledToHeight(200, Qt::SmoothTransformation);
                        QIcon temp_icon(im22);
                        item1->setIcon(temp_icon);
                    }
                    item1->setForeground(_message_color);
                    item1->setText(str + file_name);
                    _ui->listWidget_2->addItem(item1);
                }
                else if (jsonobj.contains("image"))//изображение
                {
                    qDebug() << "image";
                    int im_width = jsonobj["image_width"].toString().toInt();
                    int im_height = jsonobj["image_height"].toString().toInt();
                    QString imstr = jsonobj["image"].toString();
                    QByteArray array = QByteArray::fromBase64(imstr.toUtf8());

                    /*std::string curpath2 = QDir::currentPath().toStdString();
                    QString path2 = QString::fromStdString(curpath2.substr(0, curpath2.find_last_of("/") + 1) + "Chat_Client/image.txt");
                    QFile file(path2);
                    file.open(QIODeviceBase::WriteOnly);
                    file.write(array);
                    file.close();*/

                    QImage im((uchar *)array.data(), im_width, im_height, QImage::Format_RGB32);
                    QPixmap image = QPixmap::fromImage(im);
                    QString name = jsonobj["username"].toString();//имя
                    QString date = jsonobj["date"].toString();//дата и время сообщения
                    QString address = jsonobj["address"].toString();//ip и порт
                    QString avatarka = jsonobj["avatarka"].toString();//аватарка
                    int ava_width = jsonobj["avatarka_width"].toString().toInt();
                    int ava_height = jsonobj["avatarka_height"].toString().toInt();
                    if (address == _socket->peerAddress().toString() && name == _username)//если это ваше сообщение, то выделяется синим
                    {
                        name = "YOU";
                    }
                    if (_user_status != "NoDisturb")//проигрывать звук, если не стоит режим не беспокоить
                    {
                        std::string curpath = QDir::currentPath().toStdString();
                        QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1) + "Chat_Client/sms.wav");
                        QMediaPlayer effect;
                        effect.setSource(QUrl::fromLocalFile(path));
                        effect.play();
                        QApplication::beep();
                    }
                    QString str = "";
                    if(_show_time)
                    {
                        str = date + " (" + name + "): ";
                    }
                    else
                    {
                        str = "(" + name + "): ";
                    }
                    QListWidgetItem *item1 = new QListWidgetItem;
                    item1->setForeground(_message_color);
                    item1->setText(str);
                    _messages_map[item1] = std::make_tuple(name, date, "");
                    if (!avatarka.isEmpty())
                    {
                        QByteArray array2 = QByteArray::fromBase64(avatarka.toUtf8());
                        QImage im2((uchar *)array2.data(), ava_width, ava_height, QImage::Format_RGB32);
                        QPixmap im22 = QPixmap::fromImage(im2);
                        im22=im22.scaledToWidth(200, Qt::SmoothTransformation);
                        im22=im22.scaledToHeight(200, Qt::SmoothTransformation);
                        QIcon temp_icon(im22);
                        item1->setIcon(temp_icon);
                    }
                    _ui->listWidget_2->addItem(item1);
                    QPixmap temp = image.scaledToWidth(240, Qt::SmoothTransformation);
                    temp=temp.scaledToHeight(320, Qt::SmoothTransformation);
                    QListWidgetItem *item2 = new QListWidgetItem;
                    item2->setSizeHint(QSize(240, 320));
                    item2->setData(Qt::DecorationRole, temp);
                    _images_map.insert(item2, std::make_tuple(imstr, im_width, im_height));
                    _ui->listWidget_2->addItem(item2);
                }
                else if (jsonobj.contains("message"))//сообщение
                {
                    qDebug() << "message";
                    QString message = jsonobj["message"].toString();//сообщение
                    QString name = jsonobj["username"].toString();//имя
                    QString avatarka = jsonobj["avatarka"].toString();//аватарка
                    int ava_width = jsonobj["avatarka_width"].toString().toInt();
                    int ava_height = jsonobj["avatarka_height"].toString().toInt();
                    QString date = jsonobj["date"].toString();//дата и время сообщения
                    QString address = jsonobj["address"].toString();//ip и порт
                    if (address == _socket->peerAddress().toString() && name == _username)//если это ваше сообщение, то выделяется синим
                    {
                        name = "YOU";
                    }
                    if (_user_status != "NoDisturb")//проигрывать звук, если не стоит режим не беспокоить
                    {
                        std::string curpath = QDir::currentPath().toStdString();
                        QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1) + "Chat_Client/sms.wav");
                        QMediaPlayer effect;
                        effect.setSource(QUrl::fromLocalFile(path));
                        effect.play();
                        QApplication::beep();
                    }
                    QString str = "";
                    if(_show_time)
                    {
                        str = date + " (" + name + "): ";
                    }
                    else
                    {
                        str = "(" + name + "): ";
                    }
                    QListWidgetItem *item1 = new QListWidgetItem;
                    if (!avatarka.isEmpty())
                    {
                        QByteArray array = QByteArray::fromBase64(avatarka.toUtf8());
                        QImage im((uchar *)array.data(), ava_width, ava_height, QImage::Format_RGB32);
                        QPixmap im22 = QPixmap::fromImage(im);
                        im22=im22.scaledToWidth(200, Qt::SmoothTransformation);
                        im22=im22.scaledToHeight(200, Qt::SmoothTransformation);
                        QIcon temp_icon(im22);
                        item1->setIcon(temp_icon);
                    }
                    item1->setForeground(_message_color);
                    item1->setText(str + message);
                    _messages_map[item1] = std::make_tuple(name, date, message);
                    _ui->listWidget_2->addItem(item1);
                }
                else if(jsonobj.contains("is_new"))//информация о новом/ушедшем пользователе
                {
                    qDebug() << "user";
                    bool is_new = jsonobj["is_new"].toBool();//новый ли пользователь
                    QString name = jsonobj["username"].toString();//имя
                    QString ip = jsonobj["ip"].toString();//ip
                    QString time = jsonobj["time"].toString();//время когда пользователь присоединился
                    QString status = jsonobj["status"].toString();//статус
                    QString avatarka = jsonobj["avatarka"].toString();//аватарка
                    int ava_width = jsonobj["avatarka_width"].toString().toInt();
                    int ava_height = jsonobj["avatarka_height"].toString().toInt();
                    if (is_new)//если пользователь зашёл в чат
                    {
                        qDebug() << "new";
                        QListWidgetItem *item = new QListWidgetItem;
                        item->setForeground(_message_color);
                        item->setText(name);
                        if (!avatarka.isEmpty())
                        {
                            QByteArray array = QByteArray::fromBase64(avatarka.toUtf8());
                            QImage im((uchar *)array.data(), ava_width, ava_height, QImage::Format_RGB32);
                            QPixmap im22 = QPixmap::fromImage(im);
                            im22=im22.scaledToWidth(200, Qt::SmoothTransformation);
                            im22=im22.scaledToHeight(200, Qt::SmoothTransformation);
                            QIcon temp_icon(im22);
                            item->setIcon(temp_icon);
                        }
                        _users_vector.push_back(new Users(name, ip, time, status, avatarka, item));
                        _ui->listWidget->addItem(item);
                    }
                    else//если пользователь вышел из чата
                    {
                        qDebug() << "leave";
                        auto temp = std::make_tuple(name, ip, time, status);
                        for (auto i = _users_vector.begin(); i != _users_vector.end(); i++)
                        {
                            auto temp_i = std::make_tuple((*i)->name, (*i)->ip, (*i)->date, (*i)->status);
                            if (temp_i == temp)
                            {
                                _ui->listWidget->removeItemWidget((*i)->element);
                                delete (*i)->element;
                                _users_vector.erase(i);
                                break;
                            }
                        }
                    }
                }
            }
}

void MainWindow::on_actionInfoAboutUserTriggered(QListWidgetItem *item)
{
    QMenu* cursorMenu = new QMenu();
    _chosen_item = item;
    QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::Down);
    QPushButton* button_info = new QPushButton("Information");
    connect(button_info, &QPushButton::clicked, this, &MainWindow::on_actionInfoAboutUserModalTriggered);
    connect(button_info, SIGNAL(clicked()), cursorMenu, SLOT(close()));
    boxLayout->addWidget(button_info);
    cursorMenu->setLayout(boxLayout);
    cursorMenu->popup(QCursor::pos());
}

void MainWindow::on_actionInfoAboutUserModalTriggered()
{
    std::tuple<QString, QString, QString> info;
    for (auto i = _users_vector.begin(); i < _users_vector.end(); i++)
    {
        if ((*i)->element == _chosen_item)
        {
            info = std::make_tuple((*i)->ip, (*i)->status, (*i)->date);
            if(!_show_ip)
            {
                std::get<0>(info) = "HIDDEN";
            }
            break;
        }
    }

    QDialog* modalDialog = new QDialog(this);
    modalDialog->setModal(true);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    QLabel *Label = new QLabel("Information about user");
    Label->setAlignment(Qt::AlignCenter);
    boxLayout1->addWidget(Label);

    QPushButton *exit_button = new QPushButton("Close");
    connect(exit_button, SIGNAL(clicked()), modalDialog, SLOT(close()));

    QLabel *ipLabel = new QLabel("IP: " + std::get<0>(info));
    QLabel *timeLabel = new QLabel("Date and time: " + std::get<1>(info));
    QLabel *statusLabel = new QLabel("Status: " + std::get<2>(info));

    boxLayout1->addWidget(ipLabel);
    boxLayout1->addWidget(timeLabel);
    boxLayout1->addWidget(statusLabel);
    boxLayout1->addWidget(exit_button);

    modalDialog->setLayout(boxLayout1);
    modalDialog->exec();
}

void MainWindow::SendMessageToServer(QString str)
{
    QJsonObject jsonobj;
    jsonobj.insert("message", str);
    jsonobj.insert("username", _username);
    jsonobj.insert("status", _user_status);
    QByteArray bArray((char*)_photo.toImage().bits(), _photo.toImage().sizeInBytes());
    jsonobj.insert("avatarka", QString::fromUtf8(bArray.toBase64()));
    jsonobj.insert("avatarka_width", QString::number(_photo.toImage().width()));
    jsonobj.insert("avatarka_height", QString::number(_photo.toImage().height()));
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    _socket->sendBinaryMessage(arrJson);
    _ui->lineEdit->clear();
}

void MainWindow::SendHistoryRequestToServer()
{
    QJsonObject jsonobj;
    jsonobj.insert("history", true);
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    _socket->sendBinaryMessage(arrJson);
    _ui->lineEdit->clear();
}

void MainWindow::SendNewUsernameOrStatusOrAvaToServer()
{
    QJsonObject jsonobj;
    jsonobj.insert("new_username", _username);
    jsonobj.insert("new_status", _user_status);
    QByteArray bArray((char*)_photo.toImage().bits(), _photo.toImage().sizeInBytes());
    jsonobj.insert("new_avatarka", QString::fromUtf8(bArray.toBase64()));
    jsonobj.insert("avatarka_width", QString::number(_photo.toImage().width()));
    jsonobj.insert("avatarka_height", QString::number(_photo.toImage().height()));
    QJsonDocument json;
    json.setObject(jsonobj);
    QByteArray arrJson(json.toJson());
    _socket->sendBinaryMessage(arrJson);
}

void MainWindow::on_actionSaveIpPortTriggered()//сохранение изменений ip и порта
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
        _ui->actionServer->setText(_ip + ":" + _port);
    }
    catch(...)
    {
        QMessageBox::warning(this, "Ошибка!", "Введён неверный порт!");
    }
}

void MainWindow::on_actionConnect_triggered()// подключение к серверу
{
    try
    {
        on_actionDisconnect_triggered();
        _socket->open(QUrl("ws://" + _ip + ":" + _port));
        this->setWindowTitle("Status: " + _user_status + " " + _ip + ":" + _port);
        _socket->setPauseMode(QAbstractSocket::PauseMode::PauseNever);
    }
    catch(...)
    {
        qDebug() << "Error, disconnecting from server";
    }
}

void MainWindow::on_actionSaveUsernameTriggered()
{
    try
    {
        if (new_username->text() != "")
        {
            _username = new_username->text();
            new_username->setText(_username);
        }
        if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
        {
            SendNewUsernameOrStatusOrAvaToServer();
        }
    }
    catch(...)
    {
        QMessageBox::warning(this, "Ошибка!", "Введён неверный никнейм!");
    }
}

void MainWindow::on_actionSaveStatusTriggered()
{
    try
    {
        if (new_status->text() != "")
        {
            _user_status = new_status->text();
            QString temp_status = _user_status;
            if (_user_status.length() > 16)
            {
                temp_status = temp_status.mid(0,16) + "...";
            }
            _ui->actionAnother->setText(temp_status);
            new_status->setText(temp_status);
            if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
            {
                SendNewUsernameOrStatusOrAvaToServer();
                this->setWindowTitle("Status: " + _user_status + " " + _ip + ":" + _port);
            }
            else
            {
                this->setWindowTitle("Status: " + _user_status);
            }
        }
        _ui->action->setChecked(false);
        _ui->action_2->setChecked(false);
        _ui->action_3->setChecked(false);
    }
    catch(...)
    {
        QMessageBox::warning(this, "Ошибка!", "Введён неверный никнейм!");
    }
}

void MainWindow::on_actionDisconnect_triggered()// отключение от сервера
{
    if (_socket->state() != QAbstractSocket::SocketState::UnconnectedState && _socket->state() != QAbstractSocket::SocketState::ClosingState)
    {
        _socket->flush();
        _socket->close();
        this->setWindowTitle("Status: " + _user_status);
        _ui->listWidget->clear();
        _ui->listWidget_2->clear();
    }
}

void MainWindow::on_actionChoosePathTriggered()// выбор пути для сохранения
{
    _path = QFileDialog::getSaveFileName(this, tr("Save"), "C:/Users", "*.xml");
    _file_name->setText(_path);
    _password = new_password->text();
}

void MainWindow::on_actionDialogClosedTriggered(int i)
{
    if (file.isOpen()) file.close();
}

void MainWindow::on_actionSave_to_XML_triggered()// сохранить историю в XML файл
{
    QDialog* modalDialog = new QDialog(this);
    modalDialog->setModal(true);
    connect(modalDialog, SIGNAL(finished(int)), this, SLOT(on_actionDialogClosedTriggered(int)));

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    QLabel* Label = new QLabel("XML");
    Label->setAlignment(Qt::AlignCenter);
    boxLayout1->addWidget(Label);

    QPushButton* save_button = new QPushButton("Write to .xml");
    connect(save_button, &QPushButton::clicked, this, &MainWindow::SendHistoryRequestToServer);
    connect(save_button, SIGNAL(clicked()), modalDialog, SLOT(close()));

    QPushButton* path_button = new QPushButton("Choose path");
    connect(path_button, &QPushButton::clicked, this, &MainWindow::on_actionChoosePathTriggered);

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

void MainWindow::SaveToXML()
{
    #ifdef CRYPTO
    OPENSSL_add_all_algorithms_conf();
    ERR_load_ERR_strings();
    ENGINE *engine_gost = ENGINE_by_id("gost");
    const EVP_CIPHER * cipher_gost = EVP_get_cipher_by_name("gost89");
    unsigned char * iv = (unsigned char * ) "12345678";
    unsigned char ciph[512];
    int ciph_len = 0;
    EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
    QByteArray ba = _password.toLocal8Bit();
    const char *pass = ba.data();
    int init = EVP_EncryptInit_ex(ctx, cipher_gost, engine_gost, pass, iv);
    #endif // CRYPTO

    QDomDocument doc;
    QDomElement passw = doc.createElement("password");
    QDomText passwText = doc.createTextNode(_password.toUtf8().toBase64(QByteArray::Base64Encoding));
    QDomElement gigablock = doc.createElement("gigablock");
    doc.appendChild(passw);
    passw.appendChild(passwText);
    doc.appendChild(gigablock);
    for(auto i: _history)
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

        #ifdef CRYPTO
        ba = QByteArray::fromHex(text.toLatin1());
        const unsigned char *result = reinterpret_cast<const unsigned char *>(ba.constData());
        int enc = EVP_EncryptUpdate(ctx, ciph, &ciph_len, result, text.length());
        log.setAttribute("content", enc);
        QDomText messageText = doc.createTextNode(enc);
        #endif // CRYPTO

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

void MainWindow::on_actionExit_triggered()// выход из приложения клиента
{
    SaveSettings();
    delete settings;
    on_actionDisconnect_triggered();
    if (file.isOpen()) file.close();
    std::string curpath = QDir::currentPath().toStdString();
    QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1) + "Chat_Client/");
    QDir dir_temp(path + "temp");
    dir_temp.removeRecursively();
    this->close();
    qApp->exit();
}

#ifdef CRYPTO
void MainWindow::onConnected()
{
    this->_sok->write(this->_name.toStdString().c_str());
    if (!this->_sok->waitForBytesWritten(3000))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to send username to the server, please check connection!");
        msgBox.exec();
    }
    else
    {
        qDebug() << this->_name;
    }
}
#endif // CRYPTO

void MainWindow::UploadSettings()
{
    settings->beginGroup("MainWindow");
    _ip = settings->value("ip", "127.0.0.1").toString();
    _port = settings->value("port", "45678").toString();
    _username = settings->value("username", "Guest").toString();
    _user_status = settings->value("status", "Available").toString();
    if (_user_status == "Available")
    {
        _ui->action->setChecked(true);
        _ui->action_2->setChecked(false);
        _ui->action_3->setChecked(false);
    }
    else if (_user_status == "NotActive")
    {
        _ui->action->setChecked(false);
        _ui->action_2->setChecked(true);
        _ui->action_3->setChecked(false);
    }
    else if(_user_status == "NoDisturb")
    {
        _ui->action->setChecked(false);
        _ui->action_2->setChecked(false);
        _ui->action_3->setChecked(true);
    }
    else
    {
        _ui->action->setChecked(false);
        _ui->action_2->setChecked(false);
        _ui->action_3->setChecked(false);
    }
    QString t = settings->value("avatarka", "").toString();
    int width = settings->value("avatarka_width", "").toString().toInt();
    int height = settings->value("avatarka_height", "").toString().toInt();
    _message_color = QColor::fromString(settings->value("message_color", "#000000").toString());
    _back_color = QColor::fromString(settings->value("back_color", "#fffff").toString());
    _show_ip = settings->value("show_ip", true).toBool();
    _ui->actionShow_sender_ip->setChecked(_show_ip);
    _show_time = settings->value("show_time", true).toBool();
    _ui->actionShow_message_time->setChecked(_show_time);
    if(!t.isEmpty())
    {
        QImage im((uchar *)qUncompress(QByteArray::fromBase64(t.toUtf8())).data(), width, height, QImage::Format_RGB32);
        _photo = QPixmap::fromImage(im);
        photo->setPixmap(_photo);
    }
    settings->endGroup();
}

void MainWindow::SaveSettings()
{
    settings->beginGroup("MainWindow");
    settings->setValue("ip", _ip);
    settings->setValue("port", _port);
    settings->setValue("username", _username);
    settings->setValue("status", _user_status);
    QByteArray bArray((char*)_photo.toImage().bits(), _photo.toImage().sizeInBytes());
    QByteArray compressDataForExport = qCompress(bArray).toBase64();
    settings->setValue("avatarka", QString::fromUtf8(compressDataForExport));
    settings->setValue("avatarka_width", QString::number(_photo.toImage().width()));
    settings->setValue("avatarka_height", QString::number(_photo.toImage().height()));
    settings->setValue("message_color", _message_color.name(QColor::HexArgb));
    settings->setValue("back_color", _back_color.name(QColor::HexArgb));
    settings->setValue("show_ip", QString::number(_show_ip));
    settings->setValue("show_time", QString::number(_show_time));
    settings->endGroup();
}

void MainWindow::on_actionServer_triggered()
{
    QDialog* modalDialog = new QDialog(this);
    modalDialog->setModal(true);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    QLabel *Label = new QLabel("Server");
    Label->setAlignment(Qt::AlignCenter);
    boxLayout1->addWidget(Label);

    QPushButton *exit_button = new QPushButton("Close");
    connect(exit_button, SIGNAL(clicked()), modalDialog, SLOT(close()));
    QPushButton *save_button = new QPushButton("Save");
    connect(save_button, SIGNAL(clicked()), this, SLOT(on_actionSaveIpPortTriggered()));

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

void MainWindow::on_actionUsername_triggered()// изменить имя пользователя
{
    QDialog* modalDialog = new QDialog(this);
    modalDialog->setModal(true);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    QLabel *Label = new QLabel("Username");
    Label->setAlignment(Qt::AlignCenter);
    boxLayout1->addWidget(Label);

    QPushButton *exit_button = new QPushButton("Close");
    connect(exit_button, SIGNAL(clicked()), modalDialog, SLOT(close()));
    QPushButton *save_button = new QPushButton("Save");
    connect(save_button, SIGNAL(clicked()), this, SLOT(on_actionSaveUsernameTriggered()));

    QBoxLayout* boxLayout2 = new QBoxLayout(QBoxLayout::LeftToRight);
    QLabel *usernameLabel = new QLabel("USERNAME: ");
    boxLayout2->addWidget(usernameLabel);
    boxLayout2->addWidget(new_username);

    boxLayout1->addLayout(boxLayout2);
    boxLayout1->addWidget(save_button);
    boxLayout1->addWidget(exit_button);

    modalDialog->setLayout(boxLayout1);
    modalDialog->exec();
}

void MainWindow::Change_Background_Color()
{
    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, _back_color);
    _ui->listWidget->setAutoFillBackground(true);
    _ui->listWidget->setPalette(pal);
    _ui->listWidget_2->setAutoFillBackground(true);
    _ui->listWidget_2->setPalette(pal);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

void MainWindow::Change_Messages_Color()
{
    for(int i = 0; i < _ui->listWidget_2->count(); ++i)
    {
        auto temp = _ui->listWidget_2->item(i);
        temp->setForeground(_message_color);
    }
}

void MainWindow::Send_Button_clicked()
{
    menu->setVisible(false);
    if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
    {
        if (button_send->text() == "Send text")
        {
            SendMessageToServer(_ui->lineEdit->text());
        }
        else if (button_send->text() == "Send image")
        {
            QString path = QFileDialog::getOpenFileName(this, tr("Open image"), "C:/Users", tr("Images (*.png *.xpm *.jpg)"));
            QPixmap image(path);
            SendImageToServer(image);
        }
        else if (button_send->text() == "Send file")
        {
            QString path = QFileDialog::getOpenFileName(this, tr("Open file"), "C:/Users");
            QFile file (path);

            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QString temp = file.fileName();
                QStringList parts = temp.split("/");
                QString file_name = parts.at(parts.size()-1);
                QByteArray file_array = file.readAll();
                SendFileToServer(file_array.toBase64(), file_name);
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Внимание!", "Соединение с сервером потеряно, попробуйте подключиться повторно!");
    }
}

void MainWindow::Show_MultiButton_clicked()
{
    menu->setVisible(true);
}

void MainWindow::SendImageToServer(QPixmap image)
{
    QByteArray bArray((char*)image.toImage().bits(), image.toImage().sizeInBytes());
    if(!bArray.isEmpty())
    {
        QJsonObject jsonobj;
        jsonobj.insert("image", QString::fromUtf8(bArray.toBase64()));
        jsonobj.insert("image_width", QString::number(image.toImage().width()));
        jsonobj.insert("image_height", QString::number(image.toImage().height()));
        jsonobj.insert("username", _username);
        jsonobj.insert("status", _user_status);
        QByteArray bArray2((char*)_photo.toImage().bits(), _photo.toImage().sizeInBytes());
        jsonobj.insert("avatarka", QString::fromUtf8(bArray2.toBase64()));
        jsonobj.insert("avatarka_width", QString::number(_photo.toImage().width()));
        jsonobj.insert("avatarka_height", QString::number(_photo.toImage().height()));
        QJsonDocument json;
        json.setObject(jsonobj);
        QByteArray arrJson(json.toJson());
        _socket->sendBinaryMessage(arrJson);
    }

    /*//testing
    QListWidgetItem *item = new QListWidgetItem;
    //QByteArray temp2 = bArray;
    QPixmap image2;
    //QImage im = QImage::fromData(temp2);
    //QImage im(qUncompress(bArray));
    //qDebug() << QString::number(bArray.size());
    //qDebug() << QString::number(bArray2.size());
    //QImage im(image.toImage().constBits(), image.toImage().width(), image.toImage().height(), image.toImage().bytesPerLine(), image.toImage().format());
    //QImage im = QImage::fromData(qUncompress(bArray));
    //image2 = QPixmap::fromImage(QImage::fromData(temp2));
    QString h = QString::fromUtf8(bArray.toBase64());
    QByteArray f = QByteArray::fromBase64(h.toUtf8());
    QImage im((uchar *)f.data(),image.toImage().width(),image.toImage().height(),QImage::Format_RGB32);
    image2 = QPixmap::fromImage(im);
    image2 = image2.scaledToWidth(240, Qt::SmoothTransformation);
    image2 = image2.scaledToHeight(320, Qt::SmoothTransformation);
    item->setSizeHint(QSize(240, 320));
    item->setData(Qt::DecorationRole, image2);
    _ui->listWidget_2->addItem(item);*/
}

void MainWindow::SendFileToServer(QString file, QString file_name)//отправка файла
{
    if(!file.isEmpty())
    {
        QJsonObject jsonobj;
        jsonobj.insert("file", file);
        jsonobj.insert("file_name", file_name);
        jsonobj.insert("username", _username);
        jsonobj.insert("status", _user_status);
        QByteArray bArray2((char*)_photo.toImage().bits(), _photo.toImage().sizeInBytes());
        jsonobj.insert("avatarka", QString::fromUtf8(bArray2.toBase64()));
        jsonobj.insert("avatarka_width", QString::number(_photo.toImage().width()));
        jsonobj.insert("avatarka_height", QString::number(_photo.toImage().height()));
        QJsonDocument json;
        json.setObject(jsonobj);
        QByteArray arrJson(json.toJson());
        _socket->sendBinaryMessage(arrJson);
    }
}

void MainWindow::on_action_triggered(bool checked)
{
    _user_status = "Available";
    _ui->action->setChecked(true);
    _ui->action_2->setChecked(false);
    _ui->action_3->setChecked(false);
    if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
    {
        SendNewUsernameOrStatusOrAvaToServer();
        this->setWindowTitle("Status: " + _user_status + " " + _ip + ":" + _port);
    }
    else
    {
        this->setWindowTitle("Status: " + _user_status);
    }
}

void MainWindow::on_action_2_triggered(bool checked)
{
    _user_status = "NotActive";
    _ui->action->setChecked(false);
    _ui->action_2->setChecked(true);
    _ui->action_3->setChecked(false);
    if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
    {
        SendNewUsernameOrStatusOrAvaToServer();
        this->setWindowTitle("Status: " + _user_status + " " + _ip + ":" + _port);
    }
    else
    {
        this->setWindowTitle("Status: " + _user_status);
    }
}

void MainWindow::on_action_3_triggered(bool checked)
{
    _user_status = "NoDisturb";
    _ui->action->setChecked(false);
    _ui->action_2->setChecked(false);
    _ui->action_3->setChecked(true);
    if (_socket->state() == QAbstractSocket::SocketState::ConnectedState)
    {
        SendNewUsernameOrStatusOrAvaToServer();
        this->setWindowTitle("Status: " + _user_status + " " + _ip + ":" + _port);
    }
    else
    {
        this->setWindowTitle("Status: " + _user_status);
    }
}

void MainWindow::on_actionAbout_program_triggered()// о программе
{
    QDialog* modalDialog = new QDialog(this);
    modalDialog->setModal(true);

    QBoxLayout* boxLayout2 = new QBoxLayout(QBoxLayout::Down);
    QLabel *authorLabel = new QLabel("Author: Artem Motyakin");
    QLabel *dateLabel = new QLabel("Creation date: 18.11.2022");
    QLabel *qt1Label = new QLabel("Qt version: 6.4.0");
    QLabel *qt2Label = new QLabel("Your Qt version: " + (QString)QT_VERSION_STR);
    QPushButton *exit_button = new QPushButton("Close");
    connect(exit_button, SIGNAL(clicked()), modalDialog, SLOT(close()));
    boxLayout2->addWidget(authorLabel);
    boxLayout2->addWidget(dateLabel);
    boxLayout2->addWidget(qt1Label);
    boxLayout2->addWidget(qt2Label);
    boxLayout2->addWidget(exit_button);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::LeftToRight);
    std::string curpath = QDir::currentPath().toStdString();
    QString path = QString::fromStdString(curpath.substr(0, curpath.find_last_of("/") + 1) + "Chat_Client/ava.png");
    QPixmap image(path);
    image = image.scaledToWidth(this->size().width()/2, Qt::SmoothTransformation);
    image = image.scaledToHeight(this->size().height()/2, Qt::SmoothTransformation);
    QLabel *imageLabel = new QLabel();
    imageLabel->setPixmap(image);
    boxLayout1->addWidget(imageLabel);
    boxLayout1->addLayout(boxLayout2);

    modalDialog->setLayout(boxLayout1);
    modalDialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
    modalDialog->exec();
}

void MainWindow::on_actionAnother_triggered(bool checked)
{
    QDialog* modalDialog = new QDialog(this);
    modalDialog->setModal(true);

    QBoxLayout* boxLayout2 = new QBoxLayout(QBoxLayout::Down);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::LeftToRight);
    QLabel* Label = new QLabel("Status: ");
    boxLayout1->addWidget(Label);
    boxLayout1->addWidget(new_status);


    QPushButton* save_button = new QPushButton("Save");
    connect(save_button, &QPushButton::clicked, this, &MainWindow::on_actionSaveStatusTriggered);
    connect(save_button, SIGNAL(clicked()), modalDialog, SLOT(close()));

    boxLayout2->addLayout(boxLayout1);
    boxLayout2->addWidget(save_button);

    modalDialog->setLayout(boxLayout2);
    modalDialog->exec();
}

void MainWindow::on_actionPhoto_triggered()
{
    QDialog* modalDialog = new QDialog(this);
    modalDialog->setModal(true);

    QBoxLayout* boxLayout1 = new QBoxLayout(QBoxLayout::Down);
    boxLayout1->addWidget(photo);

    QPushButton *load_button = new QPushButton("Load photo");
    connect(load_button, SIGNAL(clicked()), this, SLOT(on_actionOpenPhotoTriggered()));
    boxLayout1->addWidget(load_button);

    QPushButton *exit_button = new QPushButton("Close");
    connect(exit_button, SIGNAL(clicked()), modalDialog, SLOT(close()));
    boxLayout1->addWidget(exit_button);

    modalDialog->setLayout(boxLayout1);
    modalDialog->exec();
}

void MainWindow::on_actionBackground_color_triggered()
{
    QColor color = QColorDialog::getColor(_back_color, this);
    if( color.isValid())
    {
        _back_color = color;
        Change_Background_Color();
        //qDebug() << "Color Back Choosen : " << color.name();
    }
}

void MainWindow::on_actionMessage_color_triggered()
{
    QColor color = QColorDialog::getColor(_back_color, this);
    if( color.isValid())
    {
        _message_color = color;
        Change_Messages_Color();
        //qDebug() << "Color Message Choosen : " << color.name();
    }
}

void MainWindow::on_actionShow_sender_ip_triggered(bool checked)
{
    _show_ip = checked;
}

void MainWindow::on_actionShow_message_time_triggered(bool checked)
{
    _show_time = checked;
    Change_Show_Time();
}

