#ifndef STATUS_H
#define STATUS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

typedef struct{
  int single_multi;
  int avatar_select;
}state;


typedef struct {
    SDL_Rect rect;       
    SDL_Texture* tex;
    SDL_Texture* hoverTex;     // from status
    void (*onClick)(void* data); // common
    void* userData;            // from status
    int hovered;               // from players
} Button;


typedef struct{
  SDL_Rect rect;
  SDL_Texture* tex;
  int toggle;
}Frame;

SDL_Texture* ChargeTex(SDL_Renderer *ren, char* path);
Mix_Music* charSelecMusic(char* path);
void Exit(void* data);
void single_multi(SDL_Renderer* ren, Button buttons[], int numButtons, int mx, int my, int mousePressed,Mix_Chunk* btnHoverSnd);
int isInside(Button* b, int mx, int my);
void changeStatus(void* data);
void avatar_select(SDL_Renderer* ren, Button buttons[], int numButtons, int mx, int my, int mousePressed, Mix_Chunk* btnHoverSnd, Frame frame1, Frame frame2);

#endif
