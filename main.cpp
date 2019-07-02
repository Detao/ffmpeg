#include "mainwindow.h"
#include <QApplication>

extern int  environme_test();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
    environme_test();
    return a.exec();
}
