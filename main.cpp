#include "mainwindow.h"
#include <QApplication>

extern int  environme_test();
extern int ffmpeg_player();
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
   // environme_test();
    ffmpeg_player();
    return 0;//a.exec();
}
