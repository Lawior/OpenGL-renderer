#include <stdio.h>
#include <stdint.h>
#include <SDL.h>

#include <glad/glad.h>

#define NK_IMPLEMENTATION //need this for nuklear to run
#define NK_SDL_GL3_IMPLEMENTATION //need this for nuklear sdl2 
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
//The following defines taken from example program as idk what to set them anyway
//This 
#define MAX_VERTEX_MEMORY 512 * 1024 //512 kb 
#define MAX_ELEMENT_MEMORY 128 * 1024
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

    //hides the mouse in the window
    /*if (SDL_SetRelativeMouseMode(SDL_TRUE) < 0)
    {
         printf("Couldn't set mouse to Relative Mode: %s\n", SDL_GetError());
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
    //The current state of the app
    AppState app_state = APPSTATE_PLAYING;

    SDL_Window* window;
    SDL_GLContext* glcontext;
    start_SDL(&window, &glcontext);
    struct nk_context* ctx = nk_sdl_init(window); 

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    //I think this is where custom fonts go
    nk_sdl_font_stash_end();
    
    //this is for matching the window pixel coordinate space to opngl (-1, 1) coordinate space
    //TODO: Maybe this should be somewhere else? We will be calling it every time we resize the window tho
    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    renderer_init(w, h);

    //keyboard state array, allows for easier continous input (don't need to SDL_KEYDOWN SDL_KEYUP with some boolean)
    Uint8* k_state = SDL_GetKeyboardState(NULL);

    uint64_t t_new = SDL_GetPerformanceCounter();
    uint64_t t_last = 0;
    double delta_time = 0;
    int running = 1;

    // Main loop divided for logic rendering and such 
    while (running) {
        t_last = t_new;
        t_new = SDL_GetPerformanceCounter();
        delta_time = (double)(t_new - t_last) / (double)SDL_GetPerformanceFrequency(); //this should give delta time in between frames in second
        
        fps_counter(delta_time);

        SDL_Event e;
         
        //handle window events including keyboard inputs
        float mouse_y = 0, mouse_x = 0; // mouse error's accumulation
        nk_input_begin(ctx); //It sets some internal states in the nuklear, needs to be called right before event poll
        while (SDL_PollEvent(&e) != 0) {
            nk_sdl_handle_event(&e); // let nuclear process inputs first
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
                        renderer_resize(w, h);
                    }
                    break;
                }
                case SDL_MOUSEMOTION:
                {
                    //handle all mouse inputs by summing them up (They should be high frequency)
                    mouse_y += e.motion.yrel;
                    mouse_x += e.motion.xrel;
                    break;
                }
            }
        }
        nk_sdl_handle_grab(); //Handles grabbable menus, which i will propably use 
        nk_input_end(ctx); //also sets some states but after the input is processed
        // move camera according with summed mouse inputs

        renderer_camera_rotate(mouse_x, mouse_y);
        if(k_state[SDL_SCANCODE_W]) renderer_camera_move_delta_forward(delta_time);
        if(k_state[SDL_SCANCODE_A]) renderer_camera_move_delta_left(delta_time);
        if(k_state[SDL_SCANCODE_S]) renderer_camera_move_delta_back(delta_time);
        if(k_state[SDL_SCANCODE_D]) renderer_camera_move_delta_right(delta_time);
        if(k_state[SDL_SCANCODE_LSHIFT] || k_state[SDL_SCANCODE_RSHIFT]) renderer_camera_move_delta_down(delta_time);
        if(k_state[SDL_SCANCODE_SPACE]) renderer_camera_move_delta_up(delta_time);

        //This is ripped from the example nuklear sdl program for testing
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;
            struct nk_colorf bg;
            bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                printf("button pressed!\n");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 22, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);
        
        renderer_render(); //main rendering done here
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY); //menu rendered here
        
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