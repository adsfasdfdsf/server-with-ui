#ifndef REGISTRATIONMODAL_H
#define REGISTRATIONMODAL_H

#include <QDialog>

namespace Ui {
class RegistrationModal;
}

class RegistrationModal : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationModal(QWidget *parent = nullptr);
    ~RegistrationModal();

    QString getName() const;
private:
    Ui::RegistrationModal *ui;
};

#endif // REGISTRATIONMODAL_H
