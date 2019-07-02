#include <QDebug>

#ifdef _WIN32
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL.h"
}
#else
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#ifdef __cplusplus
};
#endif
#endif


int ffmpeg_player()
{
    AVFormatContext *pFormatCtx;
    int i ,videoStreamIndex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    /*init ffmpeg and net module*/
    avformat_network_init();
    av_register_all();
    pFormatCtx =avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0)
    {
        qDebug<<"open input stream filed \n";
        return -1;
    }
    if(avformat_find_stream_info(&pFormatCtx,NULL)<0)
    {
        qDebug<<"find stream info failed \n";
        return -1;
    }
    videoStreamIndex=-1;

    for(i = 0;i < pFormatCtx->nb_streams;i++){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex =i;
            break;
        }
    }
    if(videoStreamIndex == -1)
    {
        qDebug<<"find video stream failed \n";
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        qDebug<<"find decoder filed \n";
        return -1;

    }
    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0)
    {
        qDebug <<"open codec fialed \n";
        return -1;
    }
    avcodec_find_decoder();
    avcodec_open2()
    return 0;
}
int SDL_Create()
{
    SDL_Window *screen;
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER)
}
