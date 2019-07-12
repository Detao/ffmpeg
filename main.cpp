#include "mainwindow.h"
#include <QApplication>
#include "pcm_decode.h"

extern int  environme_test();
extern int ffmpeg_player();
extern int audio_player(char *url);
int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    char *wavFilePath="../../source/record.wav";
    char *pcmFilePath="../../source/record.pcm";
    char *pcmLFilePath="../../source/record_l.pcm";
    char *pcmRFilePath="../../source/record_r.pcm";
    char *mp3FilePath = "../../source/1.mp3";
//    MainWindow w;
//    w.show();
   // environme_test();
 //   ffmpeg_player();
    //wavtopcm(wavFilePath,pcmFilePath);
    //simplest_pcm32Be_split(pcmFilePath,pcmLFilePath,pcmRFilePath);
    audio_player(mp3FilePath);
    return 0;//a.exec();
}
