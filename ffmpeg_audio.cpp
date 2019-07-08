/* decode and play audio stream*/

#include<QDebug>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"

//SDL
#include "SDL.h"

}

#define MAX_AUDIO_FRAME_SIZE  192000 //1 second of 48khz 32bit audio
int audio_player(char *url)
{
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    int i,audioStream;

    av_register_all();
    avformat_network_init();

    pFormatCtx=avformat_alloc_context();
    if(avformat_open_input(&pFormatCtx,url,NULL,NULL)!=0)
    {
        qDebug()<<"open input stream filed \n";
        return -1;
    }
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        qDebug()<<"find stream info failed \n";
        return -1;
    }
    av_dump_format(pFormatCtx,0,url,false);

    audioStream=-1;


    for(i = 0;i < pFormatCtx->nb_streams;i++){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO);
        {
            audioStream =i;
            break;
        }
    }
    if(audioStream  == -1)
    {
        qDebug()<<"find audio stream failed \n";
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[audioStream]->codec;
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

#if OUTPUT_PCM

#endif
    AVPacket * packet=(AVPacket *)malloc(sizeof(AVPacket));
    av_init_packet(packet);

    //out Audio Param
    uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;
    //AAC:1024 MP3:1152
    int out_nb_samples=pCodecCtx->frame_size;
    AVSampleFormat out_sample_fmt =AV_SAMPLE_FMT_S16;
    int out_sample_rate=44100;
    int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
    //out buffer size
    int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels,out_nb_samples,out_sample_fmt,1);

    uint8_t *out_buffer= (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);

    AVFrame * pFrame;

    pFrame =av_frame_alloc();
#if USE_SDL

#endif
    qDebug()<<"Bitrate: \t "<< pFormatCtx->bit_rate<<"\n";
    qDebug()<<"Decoder Name: \t"<<pCodecCtx->codec->long_name<<"\n";
    qDebug()<<"Channels: "<<pCodecCtx->channels<<"\n";
    qDebug()<<"Sample per Second "<<pCodecCtx->sample_rate<<"\n";

    uint32_t ret,len=0;
    int got_picture;
    int index=0;

    int64_t in_channel_layout=av_get_default_channel_layout(pCodecCtx->channels);
    //Swr
    struct SwrContext * au_convert_ctx;
    au_convert_ctx =swr_alloc();
    au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,out_channel_layout,out_sample_fmt,out_sample_rate,
                                      in_channel_layout,pCodecCtx->sample_fmt,pCodecCtx->sample_rate,0,NULL);
    swr_init(au_convert_ctx);
    SDL_PauseAudio(0);


    while(av_read_frame(pFormatCtx,packet)>=0){
        if(packet->stream_index==audioStream){
            ret=avcodec_decode_audio4(pCodecCtx,pFrame,&got_picture,packet);
            if(ret<0){
                qDebug()<<"Error in decoding audio frame\n";
                return -1;
            }
            if(got_picture>0){
                swr_convert(au_convert_ctx,&out_buffer,MAX_AUDIO_FRAME_SIZE,(const uint8_t **)pFrame->data,pFrame->nb_samples);
#if OUTPUT_PCM
#endif

                index++;
            }
        }
        av_free_packet(packet);
    }


    swr_free(&au_convert_ctx);




    av_free(out_buffer);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    return 0;
}
