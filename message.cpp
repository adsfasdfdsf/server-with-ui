#include "message.h"
#include "ui_message.h"

Message::Message(QString name, QString text, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Message)
{
    ui->setupUi(this);
    ui->name->setText(name);
    ui->text->setText(text);
}

Message::~Message()
{
    delete ui;
}
