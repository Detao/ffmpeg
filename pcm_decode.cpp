#include "pcm_decode.h"
#include <stdlib.h>
#include <QDebug>

int simplest_pcm32Be_split(char *url,char *l_url,char *r_url){
    FILE *fp=fopen(url,"rb+");
    FILE *fp1=fopen(l_url,"wb+");
    FILE *fp2=fopen(r_url,"wb+");

    unsigned char *sample=(unsigned char *)malloc(8);

    while(!feof(fp)){
        fread(sample,1,8,fp);
        //L
        fwrite(sample,1,4,fp1);
        //R
        fwrite(sample+4,1,4,fp2);
    }

    free(sample);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    return 0;
}

int wavtopcm(char *url,char*dst_url)
{
    _WavHeader pWavHeadr;
    FILE *fp =NULL;
    char *filepaths;
    int DataSize;
    unsigned char *pcmbuffer = (unsigned char*)malloc(8);
    unsigned char *wavHearderBuffer = (unsigned char *)malloc(44);
    if(wavHearderBuffer==NULL||pcmbuffer==NULL)
    {
        qDebug()<<"Buffer Create failed\n";
    }
    memset(wavHearderBuffer,0,44);
    FILE *fp1=fopen(dst_url,"wb+");
    if(fp1 == NULL){
        qDebug()<<"open"<<dst_url <<"failed \n";
        return -1;
    }
    if((fp = fopen(url,"rb")) == NULL)
    {
        qDebug()<<"open"<<url <<"failed \n";
        return -1;
    }
    DataSize = fread(wavHearderBuffer,1,44,fp);
    if(DataSize>0){
        /*RIFF*/
        qDebug()<<QString::number((pWavHeadr.riff.ChunkID=*((unsigned int*)wavHearderBuffer)),16);
        qDebug()<<(pWavHeadr.riff.ChunkSize=*((unsigned int*)wavHearderBuffer+4));
        qDebug()<<(pWavHeadr.riff.Format=*((unsigned int*)wavHearderBuffer+8));
        /*FMT*/
        qDebug()<<(pWavHeadr.fmt.SubChunkID=*((unsigned int*)wavHearderBuffer+12));
        qDebug()<<(pWavHeadr.fmt.SUBChunkSize=*((unsigned int*)wavHearderBuffer+16));
        qDebug()<<(pWavHeadr.fmt.AudioFormat=*((unsigned short*)wavHearderBuffer+20));
        qDebug()<<(pWavHeadr.fmt.NumChannels=*((unsigned short*)wavHearderBuffer+22));
        qDebug()<<(pWavHeadr.fmt.SampleRate=*((unsigned int*)wavHearderBuffer+24));
        qDebug()<< (pWavHeadr.fmt.ByteRate=*((unsigned int*)wavHearderBuffer+28));
        qDebug()<<(pWavHeadr.fmt.BlockAlign=*((unsigned short*)wavHearderBuffer+32));
        qDebug()<<(pWavHeadr.fmt.BitsPerSample=*((unsigned short*)wavHearderBuffer+34));
        /*DATA*/
        qDebug()<<(pWavHeadr.fmt.SubChunkID=*((unsigned int*)wavHearderBuffer+36));
        qDebug()<<(pWavHeadr.fmt.SUBChunkSize=*((unsigned int*)wavHearderBuffer+40));
    }
    while(!feof(fp)){
        DataSize = fread(pcmbuffer,1,8,fp);
        if(DataSize>0){
            fwrite(pcmbuffer,1,8,fp1);
        }else{
            qDebug()<<"read file failed\n";
        }
    }
    fclose(fp);
    fclose(fp1);
    free(pcmbuffer);
    free(wavHearderBuffer);
    return 0;
}
int pcmtowav()
{}
