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

#define SFM_REFRESH_EVENT       (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT         (SDL_USEREVENT + 2)

int thread_exit=0;
int thread_pause=0;

int sfp_refersh_thread(void* opaque)
{
    thread_exit = 0 ;
    thread_pause =0 ;
    while(!thread_exit){
        if(!thread_pause){
            /*sem send*/
            SDL_Event event;
            event.type=SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(40);
    }
    thread_exit=0;
    thread_pause=0;

    SDL_Event event;
    event.type=SFM_BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}


int ffmpeg_player()
{
    AVFormatContext *pFormatCtx;
    int i ,videoStreamIndex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    /*Frame*/
    AVFrame *pFrame ,*pFrameYUV;
    unsigned char * out_buffer;
    AVPacket *packet;
    int y_size;
    int ret ,got_picture;
    struct SwsContext *img_convert_ctx;

    //char filepath[]="../../source/bigbuckbunny_480x272.h265";
    //char filepath[]="../../source/cuc_ieschool.flv";
    char filepath[]="../../source/lupin.mp4";
    /*SDL*/
    int screen_w =0,screen_h=0;
    SDL_Window *Screen;
    SDL_Renderer *sdlRenderer;
    SDL_Texture * sdlTexture;
    SDL_Rect sdlRect;
    SDL_Thread *video_tid;
    SDL_Event event;

    /*init ffmpeg and net module*/
    avformat_network_init();
    av_register_all();
    pFormatCtx =avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0)
    {
        qDebug()<<"open input stream filed \n";
        return -1;
    }
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        qDebug()<<"find stream info failed \n";
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
        qDebug()<<"find video stream failed \n";
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        qDebug()<<"find decoder filed \n";
        return -1;

    }
    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0)
    {
        qDebug() <<"open codec fialed \n";
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameYUV= av_frame_alloc();
    out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,1));
    av_image_fill_arrays(pFrameYUV->data,pFrameYUV->linesize,out_buffer,AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,1);

    packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    qDebug()<<"--------------file information------------\n";
    av_dump_format(pFormatCtx,0,filepath,0);
    qDebug()<<"------------------------------------------\n";
    img_convert_ctx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,
                                     pCodecCtx->width,pCodecCtx->height,AV_PIX_FMT_YUV420P,SWS_BICUBIC,NULL,NULL,NULL);

    if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER)){
        qDebug()<<"SDL initialize failed ERROR:"<<SDL_GetError()<<"\n";
        return -1;
    }
    
    screen_h =600; // pCodecCtx->height;
    screen_w = 800; //pCodecCtx->width;

    Screen = SDL_CreateWindow("ffmpeg player",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,screen_w,screen_h,SDL_WINDOW_OPENGL);
    if(!Screen){
        qDebug()<<"SDL Create Window failed ERROR:"<<SDL_GetError()<<"\n";
        return -1;
    }
    sdlRenderer = SDL_CreateRenderer(Screen,-1,0);
    sdlTexture = SDL_CreateTexture(sdlRenderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);

    sdlRect.x=0;
    sdlRect.y=0;
    sdlRect.w=screen_w;
    sdlRect.h=screen_h;

//    while(av_read_frame(pFormatCtx,packet)>=0){
//        if(packet->stream_index == videoStreamIndex){
//            ret = avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
//            if(ret < 0){
//                qDebug()<<"Decode Error.\n";
//                return -1;
//            }
//            if(got_picture){
//                sws_scale(img_convert_ctx,(const unsigned char * const *)pFrame->data,pFrame->linesize,0,pCodecCtx->height,
//                          pFrameYUV->data,pFrame->linesize);

//                SDL_UpdateYUVTexture(sdlTexture,&sdlRect,
//                                     pFrameYUV->data[0],pFrameYUV->linesize[0],
//                                     pFrameYUV->data[1],pFrameYUV->linesize[1],
//                                     pFrameYUV->data[2],pFrame->linesize[2]);
//                SDL_RenderClear(sdlRenderer);
//                SDL_RenderCopy(sdlRenderer,sdlTexture,NULL,&sdlRect);
//                SDL_RenderPresent(sdlRenderer);

//                SDL_Delay(100);

//            }
//        }
//        av_free_packet(packet);
//    }
//    while(1){
//        ret = avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
//        if(ret < 0)
//            break;
//        if(!got_picture)
//            break;
//        sws_scale(img_convert_ctx,(const unsigned char * const *)pFrame->data,pFrame->linesize,0,pCodecCtx->height,
//                  pFrameYUV->data,pFrame->linesize);

//        SDL_RenderClear(sdlRenderer);
//        SDL_RenderCopy(sdlRenderer,sdlTexture,NULL,&sdlRect);
//        SDL_RenderPresent(sdlRenderer);

//        SDL_Delay(100);
//    }
    packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    video_tid = SDL_CreateThread(sfp_refersh_thread,"refresh_thead",NULL);
    while(1){
        SDL_WaitEvent(&event);
        if(event.type == SFM_REFRESH_EVENT){
            while(1){
                if(av_read_frame(pFormatCtx,packet)<0){
                    thread_exit=1;
                }
                if(packet->stream_index==videoStreamIndex)
                    break;
            }
            ret = avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
            if(ret<0){
                qDebug()<<"Decode ERROR";
                return -1;
            }
            if(got_picture){
                sws_scale(img_convert_ctx,(const unsigned char * const *)pFrame->data,
                          pFrame->linesize,0,pFrame->height,pFrameYUV->data,pFrameYUV->linesize);
                SDL_UpdateTexture(sdlTexture,NULL,pFrameYUV->data[0],pFrameYUV->linesize[0]);
                SDL_RenderClear(sdlRenderer);
                SDL_RenderCopy(sdlRenderer,sdlTexture,NULL,NULL);
                SDL_RenderPresent(sdlRenderer);


            }
            av_free_packet(packet);
        }else if(event.type == SDL_KEYDOWN){
            if(event.key.keysym.sym == SDLK_SPACE)
                thread_pause=!thread_pause;
        }else if(event.type == SDL_QUIT){
            thread_exit =1;
        }else if(event.type == SFM_BREAK_EVENT){
            break;
        }
    }
    sws_freeContext(img_convert_ctx);

    SDL_Quit();

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);


    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}

