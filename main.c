#include <stdio.h>
#include <stdint.h>
#include <SDL.h>

#include <glad/glad.h>

#define NK_IMPLEMENTATION //need this for nuklear to run
#define NK_SDL_GL3_IMPLEMENTATION //need this for nuklear sdl2 
#include "nuklear/nuklear_macros.h"
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"

#include "3d_engine/renderer.h"
#include "3d_engine/app_state.h"

// This will handle the main state transition
// I was thinking of implementing but for this project i am not planning a lot of different input states

void start_SDL(SDL_Window** window, SDL_GLContext** glcontext)
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    // setting version to 4.1, may reduce it for comaptibility 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // deprecated functionality is disabled
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //id if i will use this but this gives 8 bits that could be used to hide certain pixels
    //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


    *window = SDL_CreateWindow(
        "Retard Renderer",
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_OPENGL 
        );

    if (*window == NULL) {
        printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }


    *glcontext = SDL_GL_CreateContext(*window);
    if (*glcontext == NULL) {
        printf("Some issue creating OpenGL context. SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    /*if(SDL_GL_SetSwapInterval(-1) < 0)
    {
        printf("Adaptive VSync not supported");
        if (SDL_GL_SetSwapInterval(1) < 0) 
        {
            printf("VSync not supported: %s\n", SDL_GetError());
        }
    }*/

    //initialize glad for the acces to opengl stuff
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        exit(1);
    }
    //enables depth z test on the pixels
    glEnable(GL_DEPTH_TEST);
    
}

void fps_counter(double delta_time)
{
    static double timer=0;
    static double frame_count=0;

    timer += delta_time;
    frame_count++;

    if (timer >= 1.0f) { // Every 1 second
        printf("FPS: %.2f (ms: %.6f)\n", frame_count / timer, (timer / frame_count) * 1000.0);
        timer = 0;
        frame_count = 0;
    }
}


int main(int argc, char *argv[]) 
{

    SDL_Window* window;
    SDL_GLContext* glcontext;
    start_SDL(&window, &glcontext);
    struct nk_context* ctx = nk_sdl_init(window);

    //The current state of the app
    AppState app_state;
    start_state(&app_state, APPSTATE_PLAYING);
    AppData app_data;

    //this is for matching the window pixel coordinate space to opngl (-1, 1) coordinate space
    //TODO: Maybe this should be somewhere else? We will be calling it every time we resize the window tho
    SDL_GL_GetDrawableSize(window, &app_data.w, &app_data.h);
    renderer_init(app_data.w, app_data.h);

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    //I think this is where custom fonts go
    nk_sdl_font_stash_end();
    

    //keyboard state array, allows for easier continous input (don't need to SDL_KEYDOWN SDL_KEYUP with some boolean)

    uint64_t t_new = SDL_GetPerformanceCounter();
    uint64_t t_last = 0;
    int running = 1;

    app_data.ctx = ctx;
    app_data.k_state = SDL_GetKeyboardState(NULL);

    // Main loop divided for logic rendering and such 
    while (running) {
        t_last = t_new;
        t_new = SDL_GetPerformanceCounter();
        app_data.delta = (double)(t_new - t_last) / (double)SDL_GetPerformanceFrequency(); //this should give delta time in between frames in second
        
        fps_counter(app_data.delta);

        SDL_Event e;

        //handle window events including keyboard inputs
        nk_input_begin(ctx); //It sets some internal states in the nuklear, needs to be called right before event poll
        while (SDL_PollEvent(&e) != 0) {
            APPSTATE_TABLE[app_state].handle_input(&app_data, &e);
            switch(e.type)
            {
                case (SDL_QUIT):
                {
                    running = 0;
                    break;
                }
                case SDL_WINDOWEVENT:
                {
                    if(e.window.event == SDL_WINDOWEVENT_RESIZED
                    || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        int w, h;
                        SDL_GL_GetDrawableSize(window, &w, &h);
                        app_data.w = w;
                        app_data.h = h;
                        renderer_resize(w, h);
                    }
                    break;
                }
                case SDL_KEYDOWN:
                {
                    //handle state transition here as we only have 2 states
                    if (e.key.keysym.sym == SDLK_ESCAPE) 
                    {
                        if(app_state == APPSTATE_PLAYING) app_state_transition(&app_state, APPSTATE_MENU);
                        else if (app_state == APPSTATE_MENU) app_state_transition(&app_state, APPSTATE_PLAYING);
                    }
                    break;
                }
            }
        }
        nk_sdl_handle_grab(); //Handles grabbable menus, which i will propably use 
        nk_input_end(ctx); //also sets some states but after the input is processed

        APPSTATE_TABLE[app_state].update(&app_data);
        
        APPSTATE_TABLE[app_state].render(&app_data);
        //Update the SDL window with the buffer (swapping the buffers)
        SDL_GL_SwapWindow(window);
    }

    //Cleanup
    nk_sdl_shutdown();
    renderer_end();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}