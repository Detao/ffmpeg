#include <QDebug>

#include "SDL.h"

#undef main

extern "C"
{

    #include "libavcodec/avcodec.h"
}


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 800

char * image_path = "../../image/1.bmp";
int environme_test()
{

    qDebug()<<"hello world!\n";
    qDebug()<<avcodec_configuration();


    SDL_Window *gwindows =NULL;
    SDL_Surface *gScreenSurface = NULL;
    SDL_Surface *gHelloworld =NULL;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        qDebug()<<"Windows could not created SDL_ERROR:" << SDL_GetError()<<"\n";
        return -1;
    }

    gwindows = SDL_CreateWindow("SHOW BMP",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                SCREEN_WIDTH,
                                SCREEN_HEIGHT,
                                SDL_WINDOW_SHOWN);
    if(gwindows == NULL)
    {
        qDebug()<< "Windows could not created SDL_ERROR:" << SDL_GetError()<<"\n";
        return -1;
    }
    gScreenSurface = SDL_GetWindowSurface(gwindows);

    gHelloworld = SDL_LoadBMP(image_path);

    if(gHelloworld == NULL)
    {
        qDebug()<< "Unable load image:" << "SDL_ERROR"<<SDL_GetError()<<"\n";
        return -1;
    }

    SDL_BlitSurface(gHelloworld,NULL,gScreenSurface, NULL);
    SDL_UpdateWindowSurface(gwindows);

    SDL_Delay(2000);

    SDL_FreeSurface(gHelloworld);
    gHelloworld = NULL;

    SDL_DestroyWindow(gwindows);
    gwindows = NULL;

    SDL_Quit();


    return 0;
}
