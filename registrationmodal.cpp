#include "registrationmodal.h"
#include "ui_registrationmodal.h"

RegistrationModal::RegistrationModal(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegistrationModal)
{
    ui->setupUi(this);
    connect(ui->accept, &QPushButton::clicked, this, &QDialog::accept);
}

RegistrationModal::~RegistrationModal()
{
    delete ui;
}

QString RegistrationModal::getName() const
{
    return ui->nameInput->text();
}
