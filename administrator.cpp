#include "administrator.h"
#include "./ui_Administrator.h"
#include "message.h"
#include "registrationmodal.h"
#include <QScrollBar>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include "user.h"
#include <QBoxLayout>
#include <QTcpServer>
#include <QtSql>

Administrator::Administrator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Administrator)
{
    ui->setupUi(this);
    modal_ptr = new RegistrationModal(this);
    connect(modal_ptr, &QDialog::accepted, this, &Administrator::onRegistered);
    connect(ui->buttonSend, &QPushButton::clicked, this, &Administrator::onSendMessage);
    modal_ptr->exec();
}

Administrator::~Administrator()
{
    delete ui;
}

void Administrator::addMessage(Message *message)
{
    auto bar = ui->scrollArea->verticalScrollBar();
    if (bar->value() == bar->maximum()){
        ui->chat->layout()->addWidget(message);
        ui->chat->adjustSize();
        qApp->processEvents();
        bar->setValue(bar->maximum());
    }else{
        ui->chat->layout()->addWidget(message);
    }
}

void Administrator::addUser(user* user)
{
    dynamic_cast<QBoxLayout*>(ui->users->layout())->insertWidget(0, user);
}

void Administrator::message_to(QString name)
{
    qDebug() << name;
}



void Administrator::onNewMessage(){
    auto socket_ptr = dynamic_cast<QTcpSocket*>(sender());
    while(socket_ptr->bytesAvailable() > 0){
        auto msg = socket_ptr->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(msg);
        QJsonObject content = doc.object();
        QJsonArray contents = content["contents"].toArray();
        for(size_t i = 0; i < contents.size(); ++i){
            QJsonObject json = contents[i].toObject();
            if (json["mode"].toString() == "setName"){
                clients[socket_ptr] = json["name"].toString();
                QJsonArray arr;
                QString user_name = json["name"].toString();
                users[user_name] = new user(user_name, this);
                addUser(users[user_name]);
                arr.append(json["name"].toString());
                QJsonObject userlist;
                userlist.insert("mode", "add_user");
                userlist.insert("names", arr);
                QJsonObject content;
                QJsonArray contarr;
                contarr.append(userlist);
                QJsonObject obj;
                obj.insert("contents", contarr);
                QJsonDocument doc(obj);
                QString str = doc.toJson(QJsonDocument::Compact);
                for (auto cl = clients.keyBegin(); cl != clients.keyEnd(); ++cl){
                    if(*cl != socket_ptr){
                        (*cl)->write(str.toUtf8());
                        qDebug() << "name sent";
                    }
                }
            }else{
                QJsonArray messages = json["messages"].toArray();
                for (const auto& i: messages){
                    QJsonObject newobj = i.toObject();
                    QSqlQuery query;
                    query.prepare("INSERT INTO messages (name, message) VALUES (:name, :message)");
                    query.bindValue(":name", newobj["name"].toString());
                    query.bindValue(":message", newobj["message"].toString());
                    qDebug() << "try to print";
                    addMessage(new Message(newobj["name"].toString(), newobj["message"].toString()));
                    if (!query.exec()){
                        qDebug() << query.lastError().text();
                    }
                    qDebug() << newobj["name"].toString() << ": " << newobj["message"].toString();
                    for (auto cl = clients.keyBegin(); cl != clients.keyEnd(); ++cl){
                        (*cl)->write(msg);
                        qDebug() << "message sent";
                    }
                }
            }
        }
    }
}







void Administrator::onNewConnection()
{
    auto clsocket = server_ptr->nextPendingConnection();
    qDebug() << "new connection";
    connect(clsocket, &QTcpSocket::readyRead,
            this, &Administrator::onNewMessage);
    connect(clsocket, &QTcpSocket::disconnected,
            this, &Administrator::onDisconnect);

    QSqlQuery query;
    query.exec("SELECT * FROM messages");
    QJsonArray arr;
    QString clname;
    QString message;//рассылаем все прошлые сообщения новому пользователю
    while (query.next()){
        QJsonObject list;
        clname = query.value(1).toString();
        message = query.value(2).toString();
        list.insert("name", clname);
        list.insert("message", message);
        arr.push_back(list);
    }
    QJsonObject json;
    json.insert("mode", "message");
    json.insert("messages", arr);
    QJsonArray arr2;
    for(auto i = clients.begin(); i != clients.end(); ++i){
        arr2.append(*i);
    }
    arr2.append(name);
    QJsonObject userlist;
    userlist.insert("mode", "add_user");
    userlist.insert("names", arr2);
    QJsonObject content;//рассылаем имена пользователей в сети новому пользователю
    QJsonArray contarr;
    contarr.append(json);
    contarr.append(userlist);
    QJsonObject obj;
    obj.insert("contents", contarr);
    QJsonDocument doc(obj);
    QString str = doc.toJson(QJsonDocument::Compact);
    clsocket->write(str.toUtf8());
}

void Administrator::onDisconnect(){
    auto socket_ptr = dynamic_cast<QTcpSocket*>(sender());
    QString disconnected_name = clients[socket_ptr];
    qDebug() << "Client: " << clients[socket_ptr] << " has left";
    clients.remove(socket_ptr);
    socket_ptr->close();
    QJsonObject obj;
    QJsonArray arr;
    QJsonObject discobj;
    discobj.insert("mode", "remove_user");
    discobj.insert("name", disconnected_name);
    arr.append(discobj);
    obj.insert("contents", arr);
    QJsonDocument doc(obj);
    QString json = doc.toJson(QJsonDocument::Compact);
    for(auto i = clients.keyBegin(); i != clients.keyEnd(); ++i){
        (*i)->write(json.toUtf8());
    }
    delete users[disconnected_name];
    users.remove(disconnected_name);
}


void Administrator::onRegistered()
{
    server_ptr = new QTcpServer;
    name = modal_ptr->getName();
    connect(server_ptr, &QTcpServer::newConnection,
            this, &Administrator::onNewConnection);
    if (!server_ptr->listen(QHostAddress::Any, 1234)){
        qDebug() << "Listen failed";
        exit(1);
    }
    qDebug() << "Listen success";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("chat.db");
    db.open();
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS messages (id INTEGER PRIMARY KEY, name STRING, message STRING)")){
        qDebug() << query.lastError().text();
    }
    query.exec("SELECT * FROM messages");
    QString clname;
    QString message;
    while (query.next()){
        clname = query.value(1).toString();
        message = query.value(2).toString();
        addMessage(new Message(query.value(1).toString(), query.value(2).toString()));
    }
}


void Administrator::onSendMessage()
{
    if (ui->inputText->toPlainText() == ""){
        return;
    }
    QString msg = toJsonMsg();
    QSqlQuery query;
    query.prepare("INSERT INTO messages (name, message) VALUES (:name, :message)");
    query.bindValue(":name", name);
    query.bindValue(":message", ui->inputText->toPlainText());
    if (!ui->anonymus->isChecked()){
        addMessage(new Message(name, ui->inputText->toPlainText()));
    }else{
        addMessage(new Message("", ui->inputText->toPlainText()));
    }
    if (!query.exec()){
        qDebug() << query.lastError().text();
    }
    for (auto cl = clients.keyBegin(); cl != clients.keyEnd(); ++cl){
        (*cl)->write(msg.toUtf8());
        qDebug() << "message sent";
    }
    ui->inputText->clear();
}




QString Administrator::toJsonMsg() const
{
    QJsonArray arr;
    QJsonObject arrobj;
    if (!ui->anonymus->isChecked())
        arrobj.insert("name", name);
    else
        arrobj.insert("name", "");
    arrobj.insert("message", ui->inputText->toPlainText());
    arr.push_back(arrobj);
    QJsonObject message;
    message.insert("mode", "message");
    message.insert("messages", arr);
    QJsonObject obj;
    QJsonArray arr2;
    arr2.append(message);
    obj.insert("contents", arr2);
    QJsonDocument doc(obj);
    QString json = doc.toJson(QJsonDocument::Compact);
    return json;

}


QString Administrator::toJsonName() const{
    QJsonObject json;
    json.insert("mode", "setName");
    json.insert("name", name);
    QJsonObject obj;
    QJsonArray arr2;
    arr2.append(json);
    obj.insert("contents", arr2);
    QJsonDocument doc(obj);
    QString str = doc.toJson(QJsonDocument::Compact);
    return str;
}


