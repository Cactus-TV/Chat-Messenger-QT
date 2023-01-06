#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QDebug>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVector>
#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QLineEdit>
#include <string>
#include <QListWidgetItem>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFileDialog>
#include <QCheckBox>
#include <QSettings>
#include <fstream>
#include <QMediaPlayer>
#include <QImageWriter>
#include <QUuid>
#include <QImageReader>
#include <QByteArray>
#include <QBuffer>
#include <QMouseEvent>
#include <cstring>
#include "extra.h"
#include <QCryptographicHash>
#include <QDataStream>
#include <QDesktopServices>
#include <QPalette>
#include <QColorDialog>

//#define CRYPTO
#ifdef CRYPTO
#include "crypto_generator.h"
#include <QRegExp>
#include <QSoundEffect>
#include <QSslSocket>
#endif // CRYPTO

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Users
{
    QString name;
    QString ip;
    QString date;
    QString status;
    QString avatarka;
    QListWidgetItem* element;
    Users(QString name, QString ip, QString date, QString status, QString avatarka, QListWidgetItem* element)
    {
        this->name = name;
        this->ip = ip;
        this->date = date;
        this->status = status;
        this->element = element;
        this->avatarka = avatarka;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *_ui;
    QWebSocket *_socket;//tcp сокет
    QString _username;//имя пользователя
    QString _user_status;//статус пользователя
    QVector<Users*> _users_vector;//список юзеров в данном чате (имя, ip, дата и время, статус, элемент в списке)
    void SendMessageToServer(QString str);//отправка сообщения
    void SendFileToServer(QString file, QString file_name);//отправка файла
    void SendImageToServer(QPixmap image);//отправка изображения
    void SendHistoryRequestToServer();//отправка запроса на получение истории сообщений из чата
    void SendNewUsernameOrStatusOrAvaToServer();//отправка нового юзернейма или статуса или аватарки на сервер
    void SaveToXML();//сохранение в XML файл
    void SaveSettings();//выгрузка настроек в ini файл
    void UploadSettings();//загрузка настроек из ini файла
    void Change_Background_Color();
    void Change_Messages_Color();
    void Change_Show_Time();

    #ifdef CRYPTO
    void onConnected();
    void ChangeKey();
    #endif // CRYPTO

    QMap<QListWidgetItem*, std::tuple<QString, int, int>> _images_map;
    QMap<QListWidgetItem*, std::tuple<QString, QString>> _files_map;
    QMap<QListWidgetItem*, std::tuple<QString, QString, QString>> _messages_map;//name, time, input (text or file)
    QSettings *settings;
    QString _ip;
    QString _port;
    QLineEdit* new_ip;
    QLineEdit* new_port;
    QLineEdit* new_username;
    QLineEdit* new_password;
    QLineEdit* new_status;
    QLabel* _file_name;
    QListWidgetItem* _chosen_item;
    QJsonArray _history;
    QString _path;
    QString _password;
    bool _show_ip;
    bool _show_time;
    QColor _message_color;
    QColor _back_color;
    std::tuple<QString, QString> _chosen_file;
    QFile file;
    QExPushButton* button_send;
    QMenu* menu;
    QLabel* _chosen_image;
    QLabel* photo;
    QPixmap _photo;
    QAction* action_file;
    QAction* action_image;
    QAction* action_default;

private slots:
    void MenuTriggered1();
    void MenuTriggered2();
    void MenuTriggered3();
    void OpenImageFile(QListWidgetItem *item);
    void FullSizeImage();
    void SaveImage();
    void OpenFile();
    void SaveFile();
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionSave_to_XML_triggered();
    void on_actionExit_triggered();
    void on_actionServer_triggered();
    void on_actionUsername_triggered();
    void Send_Button_clicked();
    void Show_MultiButton_clicked();
    void on_action_triggered(bool checked);
    void on_action_2_triggered(bool checked);
    void on_action_3_triggered(bool checked);
    void on_actionAbout_program_triggered();
    void on_actionSaveIpPortTriggered();
    void on_actionSaveUsernameTriggered();
    void on_actionOpenPhotoTriggered();
    void on_actionInfoAboutUserTriggered(QListWidgetItem *item);
    void on_actionInfoAboutUserModalTriggered();
    void on_actionHideShowPasswordTriggered(int checked);
    void on_actionChoosePathTriggered();
    void on_actionDialogClosedTriggered(int i);
    void on_actionAnother_triggered(bool checked);
    void on_actionPhoto_triggered();
    void on_actionSaveStatusTriggered();
    void slotReadyRead(const QByteArray &arr);
    void on_actionBackground_color_triggered();
    void on_actionMessage_color_triggered();
    void on_actionShow_sender_ip_triggered(bool checked);
    void on_actionShow_message_time_triggered(bool checked);
};
#endif // MAINWINDOW_H
