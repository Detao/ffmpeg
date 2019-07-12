extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/avstring.h"


#include "SDL.h"
}

#include <qdebug.h>

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE  192000

#define SFM_REFRESH_EVENT       (SDL_USEREVENT )
#define SFM_BREAK_EVENT         (SDL_USEREVENT + 2)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

typedef struct PacketQueue{
    AVPacketList *first_pkg,*last_pkg;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond  *cond;

}PacketQueue;

typedef struct VideoPicture {
  SDL_Texture *bmp;
  int width, height; /* source height & width */
  int allocated;
} VideoPicture;

typedef struct VideoState{
    AVFormatContext *pFormatCtx;
    int VideoStream;
    int AudioStream;

    AVStream    *audio_st;
    AVCodecContext  *audio_ctx;
    PacketQueue  audioq;
    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE *3)/2];
    unsigned int  audio_buf_size;
    unsigned int  audio_buf_index;
    AVFrame audio_frame;
    AVPacket audio_packet;
    uint8_t *audio_pkt_data;
    int audio_pkt_size;

    AVStream *video_st;
    AVCodecContext  *video_ctx;
    PacketQueue videoq;
    struct SwsContext *sws_ctx;

    VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int pictq_size,pictq_rindex,pictq_windex;
    SDL_mutex   *pictq_mutex;
    SDL_cond    *pictq_cond;

    SDL_Thread  *parse_tid;
    SDL_Thread  *video_tid;

    char filename[1024];
    int quit;

}VideoState;

SDL_mutex *screen_mutex;
SDL_Renderer *sdlRenderer;
SDL_Texture * sdlTexture;

VideoState *global_video_state;
int packet_queue_init(PacketQueue *q)
{
    memset(q,0,sizeof(PacketQueue));
    q->mutex= SDL_CreateMutex();
    q->cond = SDL_CreateCond();
    return 0;
}

int packet_queue_put(PacketQueue *q, AVPacket *packet)
{
    AVPacketList *pkt;
    if(av_dup_packet(packet)<0)
    {
        return -1;
    }
    pkg=av_malloc(sizeof(AVPacketList));
    if(!pkg){
        return -1;
    }
    pkt->pkt=packet;
    pkt->next=NULL;
    SDL_mutex(q->mutex);
    if(!q->last_pkg)
        q->first_pkg=pkt;
    else
        q->last_pkg->next=pkt;
    q->last_pkg=pkt;
    q->nb_packets++;
    q->size+=pkt->pkt.size;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
    return 0;
}
int packet_queue_get(PacketQueue *q,AVPacket *packet,int block)
{
    AVPacketList *pkt;
    int ret;
    SDL_mutex(q->mutex);
    for(;;){
        if(global_video_state->quit!=0)
        {
            ret =-1;
            break;
        }
        pkt=q->first_pkg;
        if(!pkt){
            q->first_pkg=pkt->next;

            if(!q->first_pkg)
                q->last_pkg=NULL;
            q->nb_packets--;
            q->size-=pkt->pkt.size;
            *packet=pkt->pkt;
            av_free(pkt);
            ret =0;
            break;
        }else if(block!=0){
            ret=0;
            break;
        }else{
            SDL_CondWait(q->cond);
        }
    }

    return ret;
}
void vedio_display(VedioState *is)
{


}
void video_refesh_timer(void *userdata)
{
   VideoState *is=(VideoState *)userdata;
   VideoPicture *vp;

   if(is->video_st){
       if(is->pictq_size==0)
       {
           schedule_refresh(is,1);
       }else{
           vp=&is->pictq[is->pictq_rindex];
            schedule_refresh(is,40);

            video_display(is);
            if(++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE){
                is->pictq_rindex=0;
            }
            SDL_LockMutex(is->pictq_mutex);
            is->pictq_size--;
            SDL_CondSignal(is->pictq_cond);
            SDL_UnlockMutex(is->pictq_mutex);
       }

   }else{
       schedule_refresh(is,100);
    }
}
int queue_picture(VideoState *is, AVFrame *pFrame)
{
    VideoPicture *vp;
    int dst_pix_fmt;
    AVPicture pict;

    SDL_LockMutex(is->pictq_mutex);
    while(is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !is->quit){
        SDL_CondWait(is->pictq_cond,is->pictq_mutex);
    }
    SDL_UnlockMutex(is->pictq_mutex);

    if(is->quit)
        return -1;

    // windex is set to 0 initially
    vp = &is->pictq[is->pictq_windex];

    /*allocate or resize the buffer !*/
    if(!vp->bmp ||
            vp->width != is->video_ctx->width ||
            vp->height != is->video_ctx->height){

        SDL_Event event;
        vp->allocated = 0;
        alloc_picture(is);
        if(is->quit){
            return -1;
        }
    }
    /* we have a place to put our picture on the queue */
    if(vp->bmp){
        SDL
    }

    return 0;
}
int video_thread(void *arg)
{
    VideoState *is = (VideoState *)arg;
    AVPacket pkt , *packet = &pkt;
    int frameFnished;
    AVFrame *pFrame;

    pFrame = av_frame_alloc();

    for(;;){
        if(packet_queue_get(&is->videoq,packet,1)<0){
            break;
        }
        avcodec_decode_video2(is->video_ctx,pFrame,&frameFinshed,packet);
        if(frameFnished){
            if(queue_picture(is,pFrame)<0){
                break;
            }
        }
        av_free_packet(packet);
    }
    av_frame_free(&pFrame);
    return 0;
}
int stream_component_open(VideoState *is,int stream_index)
{
    AVFormatContext *pFormatCtx = is->pFormatCtx;
    AVCodecContext  *pCodecCtx =NULL;
    AVCodec *pCodec=NULL;

    SDL_AudioSpec wanted_spec,spec;

    if(stream_index <0 || stream_index >= pFormatCtx->nb_streams)
        return -1;
    pCodec =avcodec_find_decoder(pFormatCtx->streams[stream_index]->codec->codec_id);
    if(!pCodec){
        fprintf(stderr,"Unsupported codec!\n");
        return -1;
    }
    pCodecCtx= avcodec_alloc_context3(pCodec);
    if(avcodec_copy_context(pCodecCtx,pFormatCtx->streams[stream_index]->codec)!=0)
    {
        fprintf(stderr,"could't copy codec context");
        return -1;
    }
    if(pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO){
        //set audio setting from codec info
        wanted_spec.freq = pCodecCtx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = pCodecCtx->channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_spec.callback = audio_callback;
        wanted_spec.userdata =is ;

        if(SDL_OpenAudio(&wanted_spec,&spec)<0){
            fprintf(stderr,"SDL_OpenAudio :%s \n",SDL_GetError());
            return -1;
        }
    }
    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0){
        fprintf(stderr, "Unsupported codec! \n");
        return -1;
    }
    switch (pCodecCtx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->AudioStream=stream_index;
        is->audio_st = pFormatCtx ->streams[stream_index];
        is->audio_ctx = pCodecCtx;
        is->audio_buf_size = 0;
        is->audio_buf_index = 0;
        memset(&is->audio_packet,0,sizeof(is->audio_packet));
        packet_queue_init(&is->audioq);
        SDL_PauseAudio(0);
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->VideoStream = stream_index;
        is->video_st = pFormatCtx->streams[stream_index];
        is->video_ctx = pCodecCtx;
        packet_queue_init(&is->videoq);
        is->video_tid = SDL_CreateThread(video_thread,"video _thread",is);
        is->sws_ctx = sws_getContext(is->video_ctx->width,is->video_ctx->height,is->video_ctx->pix_fmt,
                                     is->video_ctx->width,is->video_ctx->height,AV_PIX_FMT_YUV420P,
                                     SWS_BILINEAR,NULL,NULL,NULL);
        break;
    default:
        break;
    }
}
int decode_thread(void *arg)
{
    VideoState *is = (VideoState *)arg;
    AVFormatContext * pFormatCtx;
    AVPacket pkt,*packet =&pkt；

    int video_index=-1;
    int audio_index=-1;
    int i;

    is->VideoStream=-1;
    is->AudioStream=-1;

    global_video_state = is;
    //open video file
    if(avformat_open_input(&pFormatCtx,is->filename,NULL,NULL)!=0)
        return -1;

    is->pFormatCtx = pFormatCtx;

    //retrieve stream infomation
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
        return -1;
    //Dump information about file onto standard error
    av_dump_format(pFormatCtx,0,is->filename,0);
    for(i=0;i<pFormatCtx->nb_streams;i++){
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO&&video_index<0){
            video_index=i;
        }
        if(pFormatCtx->streams[i]->codec->codec_tag==AVMEDIA_TYPE_AUDIO&&audio_index<0){
            audio_index=i;
        }
        if(audio_index>=0){
            stream_component_open(is,audio_index);
        }
        if(video_index>0){
            stream_component_open(is,video_index);
        }
        if(is->VideoStream<0 || is->AudioStream){
            fprintf(stderr,"%s:could not open codecs\n",is->filename);
            goto fail;
        }
        for(;;){
            if(is->quit){
                break;
            }
        }
        while(!is->quit){
            SDL_Delay(100);
        }
    }
fail:
    if(1){
        SDL_Event event;
        event.type=FF_QUIT_EVENT;
        event.user.data1=is;
        SDL_PushEvent(&event);
    }
    return 0;
}
int main(int argc, char *argv[])
{
    SDL_Event event;
    VideoState *is;

    is= av_mallocz(sizeof(VideoState));
    if(argc<2){
        return -1;
    }
    av_register_all();
    if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER)){
        qDebug()<<"SDL initialize failed ERROR:"<<SDL_GetError()<<"\n";
        return -1;
    }
    Screen = SDL_CreateWindow("ffmpeg player",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_OPENGL);
    if(!Screen){
        qDebug()<<"SDL Create Window failed ERROR:"<<SDL_GetError()<<"\n";
        return -1;
    }
    sdlRenderer = SDL_CreateRenderer(Screen,-1,0);
    sdlTexture = SDL_CreateTexture(sdlRenderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,)
    screen_mutex = SDL_CreateMutex();

    /*#include "libavutil/avstring.h" */
    av_strlcpy(is->filename,argv[1],sizeof(is->filename));
    is->pictq_mutex = SDL_CreateMutex();
    is->pictq_cond = SDL_CreateCond();

    is->parse_tid = SDL_CreateThread(decode_thread,"decode _thread",is);
    if(!is->parse_tid){
        av_free(is);
        return -1;
    }
    for(;;){
        SDL_WaitEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            is->quit=1;
            SDL_Quit();
            return 0;
            break;
           case SFM_REFRESH_EVENT:
            video_refesh_timer(event.user.data1);
        default:
            break;
        }
    }

    return 0;
}
