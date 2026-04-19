#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "quiz.h"

typedef struct {
    /* ---- Textures ---- */
    SDL_Texture *bg;
    SDL_Texture *btnQuiz;
    SDL_Texture *btnPuzzle;
    SDL_Texture *quizTitle;
    SDL_Texture *texA;
    SDL_Texture *texB;
    SDL_Texture *texC;
    SDL_Texture *texQuestion;

    /* ---- Audio ---- */
    Mix_Chunk *hoverSound;
    Mix_Music *quizMusic;

    /* ---- Rects (windowed 800x600) ---- */
    SDL_Rect quizRect;
    SDL_Rect puzzleRect;
    SDL_Rect A;
    SDL_Rect B;
    SDL_Rect C;
    SDL_Rect band;

    /* ---- State & hover flags ---- */
    int showQuiz;
    int hoverQuiz;
    int hoverPuzzle;
    int hoverA;
    int hoverB;
    int hoverC;
    int lastHover;
    int over;
    int puzzleSelected;

    // Quiz logic
    QuizQuestion questions[MAX];
    int numQuestions;
    int currentQuestionIdx;
    int result; // 1 for correct, 0 for wrong, -1 for undecided

    // Dynamic Text Textures
    SDL_Texture *texQText;
    SDL_Texture *texAText;
    SDL_Texture *texBText;
    SDL_Texture *texCText;
    TTF_Font *font;
    TTF_Font *fontSmall;
    SDL_Renderer *renderer;

    Uint32 quizStartTime;
    int    quizTimeLimit; // in ms
} Enigme;

void initEnigme        (Enigme *e, SDL_Renderer *r);
void handleEnigmeEvents(Enigme *e, SDL_Event event);
void renderEnigme      (Enigme *e, SDL_Renderer *r);
void freeEnigme        (Enigme *e);
