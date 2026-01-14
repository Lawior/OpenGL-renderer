#include <stdio.h>
#include <stdint.h>
#include <SDL.h>

#include "glad/glad.h"
#include "3d_engine/renderer.h"


void start_SDL(SDL_Window** window, SDL_GLContext** glcontext)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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
    if(SDL_GL_SetSwapInterval(-1) < 0)
    {
        printf("Adaptive VSync not supported");
        if (SDL_GL_SetSwapInterval(1) < 0) 
        {
            printf("VSync not supported: %s\n", SDL_GetError());
        }
        }

    //hides the mouse in the window
    if (SDL_SetRelativeMouseMode(SDL_TRUE) < 0)
    {
         printf("Couldn't set mouse to Relative Mode: %s\n", SDL_GetError());
    }

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
    
    //this is for matching the window pixel coordinate space to opngl (-1, 1) coordinate space
    //TODO: Maybe this should be somewhere else? We will be calling it every time we resize the window tho
    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    renderer_init(w, h);

    const Uint8* k_state = SDL_GetKeyboardState(NULL);

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
        //keyboard state array, allows for easier continous input (don't need to SDL_KEYDOWN SDL_KEYUP with some boolean) 
        //handle window events including keyboard inputs
        float mouse_y = 0, mouse_x = 0; // mouse error's accumulation
        while (SDL_PollEvent(&e) != 0) {
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
        // move camera according with summed mouse inputs
        renderer_camera_rotate(mouse_x, mouse_y);
        if(k_state[SDL_SCANCODE_W]) renderer_camera_move_delta_forward(delta_time);
        if(k_state[SDL_SCANCODE_A]) renderer_camera_move_delta_left(delta_time);
        if(k_state[SDL_SCANCODE_S]) renderer_camera_move_delta_back(delta_time);
        if(k_state[SDL_SCANCODE_D]) renderer_camera_move_delta_right(delta_time);
        if(k_state[SDL_SCANCODE_LSHIFT] || k_state[SDL_SCANCODE_RSHIFT]) renderer_camera_move_delta_down(delta_time);
        if(k_state[SDL_SCANCODE_SPACE]) renderer_camera_move_delta_up(delta_time);
        
        renderer_render();
        
        //Update the SDL window with the buffer (swapping the buffers)
        SDL_GL_SwapWindow(window);
    }
    
    //Cleanup
    renderer_end();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}