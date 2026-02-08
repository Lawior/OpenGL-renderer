#pragma once

#include <stdint.h>
#include <SDL.h>
#include <3d_engine/renderer.h>

typedef enum AppState {
    APPSTATE_MENU,
    APPSTATE_PLAYING,
    APPSTATE_COUNT //nice trick
} AppState;

typedef enum AppStateFlag {
    APPSTATEFLAG_VSYNC_ON = 1 << 0
} AppStateFlag;

#define MINEVENTBUFFER 64
//Instead of this I would need to rework the nuklear sdl implementation itself
typedef struct NuklearEventBuffer
{
    SDL_Event* events;
    unsigned count;
    unsigned capacity;
} NuklearEventBuffer;

//this is the struct that contains stuff that needs to be passed to functions in AppStateFunctions
//maybe not the best idea to just keep adding stuff to it but it works
typedef struct AppData{
    struct nk_context* ctx;
    const Uint8* k_state;
    double delta; //delta time
    unsigned w, h; //w, h size of the screen
    struct RendererContext rd_ctx;
    AppStateFlag flags;
    uint64_t render_accumulator;
    int refresh_rate;
    NuklearEventBuffer nk_event_buffer;
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

