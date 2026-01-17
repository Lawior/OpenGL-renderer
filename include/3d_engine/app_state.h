#pragma once

#include <SDL.h>
struct nk_context;

//The following defines taken from example program as idk what to set them anyway
#define MAX_VERTEX_MEMORY 512 * 1024 //512 kb 
#define MAX_ELEMENT_MEMORY 128 * 1024

typedef enum AppState {
    APPSTATE_MENU,
    APPSTATE_PLAYING,
    APPSTATE_COUNT //nice trick
} AppState;

//this is the struct that contains stuff that needs to be passed to functions in AppStateFunctions
typedef struct AppData{
    struct nk_context* ctx;
    const Uint8* k_state;
    double delta; //delta time
    unsigned w, h; //w, h size of the screen
} AppData;

typedef struct AppStateFunctions{
    void (*enter)();
    void (*handle_input)(AppData* data, SDL_Event *e);
    void (*update)(AppData* data); //used for continous updates (like continous input)
    void (*render)(AppData* data);
    void (*exit)();
} AppStateFunctions;

extern const AppStateFunctions APPSTATE_TABLE[];

void app_state_transition(AppState* current, AppState next);
void start_state(AppState* state_to_start, AppState initial_state);