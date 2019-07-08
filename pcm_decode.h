#ifndef PCM_DECODE_H
#define PCM_DECODE_H
#include <stdio.h>

typedef struct {
    int ChunkID;
    int ChunkSize;
    int Format;
}RIFFChunk;

typedef struct{
    int SubChunkID;
    int SUBChunkSize;
    short AudioFormat;
    short NumChannels;
    int SampleRate;
    int ByteRate;
    short BlockAlign;
    short BitsPerSample;

}FmtChunk;

typedef struct{
    int SubChunkID;
    int SubChunkSize;
}DataChunk;

typedef struct{
    RIFFChunk riff;
    FmtChunk fmt;
    DataChunk data;
}_WavHeader;


int simplest_pcm32Be_split(char *url,char *l_url,char *r_url);
int wavtopcm(char *url,char*dst_url);
#endif // PCM_DECODE_H
