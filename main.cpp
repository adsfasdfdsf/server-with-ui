#include "administrator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Administrator w;
    w.show();
    return a.exec();
}
