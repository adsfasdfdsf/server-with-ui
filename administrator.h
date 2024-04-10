#ifndef CLIENT_H
#define CLIENT_H
#include <QTcpSocket>

#include <QMainWindow>
class Message;
QT_BEGIN_NAMESPACE
namespace Ui {
class Administrator;
}
QT_END_NAMESPACE

class RegistrationModal;

class ipAdress;

class user;

class QTcpServer;

class Administrator : public QMainWindow
{
    Q_OBJECT
public:
    RegistrationModal* modal_ptr;
    ipAdress* ip_modal_ptr;
public:
    Administrator(QWidget *parent = nullptr);
    ~Administrator();
    void addMessage(Message* message);
    void addUser(user* user);
    void message_to(QString name);
public slots:
    void onRegistered();
    void onSendMessage();
private:
    QString companion;
    Ui::Administrator *ui;
    QTcpSocket socket;
    QString name;
    QMap<QString, user*> users;
    QTcpServer* server_ptr;
    QMap<class QTcpSocket*, QString> clients;
private:
    QString toJsonMsg() const;
    QString toJsonName() const;
private slots:
    void onNewConnection();
    void onDisconnect();
    void onNewMessage();
};
#endif // CLIENT_H
