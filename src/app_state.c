//Not really sure abotu this change but i wanted to try this design pattern even though it doesn't really do much when we only have a few states
//hopefully it will be useful

//this can be put here because of the implementation macro in main
#include "3d_engine/app_state.h" // this already has other used headers
#include "3d_engine/renderer.h"

#include "nuklear/nuklear_macros.h"
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"
//I will use this cause i want to have exit and entry functions for the state even though i may not need them here
//so in short a function that get's called but does nothing
static void dummy_func(void){int dummmy;}
static void dummy_func_2(AppData* data){int dummmy;}

static void menu_input(AppData* data, SDL_Event *e)
{
    nk_sdl_handle_event(e); // let nuclear process inputs first

}
static void menu_render(AppData* data)
{
    struct nk_context* ctx = data->ctx;
    nk_style_push_float(ctx, &ctx->style.window.border, 0.0f);
    nk_style_push_vec2(ctx, &ctx->style.window.padding, nk_vec2(0,0));
    nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0,0,0,0)));
    // 1. Draw the Dimming Layer (Fullscreen Background)
    if (nk_begin(ctx, "BackgroundDim", nk_rect(0, 0, data->w, data->h), 
        NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT)) 
    {
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
        nk_fill_rect(canvas, nk_rect(0, 0, data->w, data->h), 0, nk_rgba(0, 0, 0, 70));
    }
    nk_end(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_vec2(ctx);
    nk_style_pop_float(ctx);

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
    renderer_render(data); //main rendering done here
    nk_sdl_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY); //menu rendered here
}

static void playing_input(AppData* data, SDL_Event *e)
{
    float mouse_y = 0, mouse_x = 0; // mouse error's accumulation
    switch(e->type)
    {
        case SDL_MOUSEMOTION:
        {
            //handle all mouse inputs by summing them up (They should be high frequency)
            mouse_y += e->motion.yrel;
            mouse_x += e->motion.xrel;
            break;
        }
    }
    // move camera according with summed mouse inputs
    renderer_camera_rotate(mouse_x, mouse_y);
}

static void playing_update(AppData* data)
{
    if(data->k_state[SDL_SCANCODE_W]) renderer_camera_move_delta_forward(data->delta);
    if(data->k_state[SDL_SCANCODE_A]) renderer_camera_move_delta_left(data->delta);
    if(data->k_state[SDL_SCANCODE_S]) renderer_camera_move_delta_back(data->delta);
    if(data->k_state[SDL_SCANCODE_D]) renderer_camera_move_delta_right(data->delta);
    if(data->k_state[SDL_SCANCODE_LSHIFT] || data->k_state[SDL_SCANCODE_RSHIFT]) renderer_camera_move_delta_down(data->delta);
    if(data->k_state[SDL_SCANCODE_SPACE]) renderer_camera_move_delta_up(data->delta);
}

static void playing_on_enter()
{
    //hides the mouse in the window
    if (SDL_SetRelativeMouseMode(SDL_TRUE) < 0)
    {
         printf("Couldn't set mouse to Relative Mode: %s\n", SDL_GetError());
    }
}
static void menu_on_enter()
{
    //hides the mouse in the window
    if (SDL_SetRelativeMouseMode(SDL_FALSE) < 0)
    {
         printf("Couldn't set mouse to Regular Mode: %s\n", SDL_GetError());
    }
}

//this is a lookup table for the 
const AppStateFunctions APPSTATE_TABLE[APPSTATE_COUNT] = {
    [APPSTATE_PLAYING] = { //this makes it so even if the enum value changes the table still works
        .enter = playing_on_enter,
        .render = renderer_render,
        .update = playing_update,
        .handle_input = playing_input,
        .exit = dummy_func
    },
    [APPSTATE_MENU] = {
        .enter = menu_on_enter,
        .render = menu_render,
        .update = dummy_func_2,
        .handle_input = menu_input,
        .exit = dummy_func
    }
}; 

void start_state(AppState* state_to_start, AppState initial_state)
{
    *state_to_start = initial_state;
    APPSTATE_TABLE[initial_state].enter();
}

void app_state_transition(AppState* current, AppState next)
{
    APPSTATE_TABLE[*current].exit();
    APPSTATE_TABLE[next].enter();
    *current = next;
}

