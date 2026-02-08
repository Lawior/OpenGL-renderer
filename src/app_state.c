//Not really sure about this change but i wanted to try this design pattern even though it doesn't really do much when we only have a few states
//hopefully it will be useful
#include <stdio.h>
//this can be put here because of the implementation macro in main
#include "3d_engine/app_state.h" // this already has other used headers
#include "3d_engine/renderer.h"
#include "3d_engine/resource_manager.h"

#include "nuklear/nuklear_macros.h"
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"

//I will use this cause i want to have exit and entry functions for the state even though i may not need them here
//so in short a function that get's called but does nothing
static void dummy_func(void){int dummmy;}
static void dummy_func_2(AppData* data){int dummmy;}

static void nk_buffer_event_push(AppData* data, SDL_Event* e)
{
    NuklearEventBuffer* buf = &data->nk_event_buffer;
    buf->count++;
    if(buf->count > buf->capacity)
    {
        buf->capacity = buf->capacity == 0 ? MINEVENTBUFFER : buf->capacity*2;
        buf->events = realloc(buf->events, buf->capacity * sizeof(SDL_Event));
        if(buf->events == NULL)
        {
            printf("Fatal Error: Buffer for events passed to nuklear couldn't be created\n");
            buf->capacity = 0;
            return;
        }
    }
    buf->events[buf->count-1] = *e;
}

//Here just for showcasing some stuff
void physic(double delta_time)
{
    //shouldn't be doing this every loop but it's simple
    unsigned gnome_index1 = rm_get_named_model_index("gnome.glb");
    unsigned gnome_index2 = rm_get_named_model_index("gnome2");
    unsigned gnome_index3 = rm_get_named_model_index("gnome3");

    Model* buffer = rm_get_model_buffer();
    
    //below just some funny stuff
    static int direction = 1;
    if(buffer[gnome_index1].pos[1] < 10 && direction) {buffer[gnome_index1].pos[1] += delta_time*10; direction = 1;}
    else if(buffer[gnome_index1].pos[1] > -10) {buffer[gnome_index1].pos[1] -= delta_time*10; direction = 0;}
    else direction = 1;
    buffer[gnome_index1].angle[1] = buffer[gnome_index1].angle[1] > 360.0f ? 0 : buffer[gnome_index1].angle[1];
    buffer[gnome_index1].angle[1] = buffer[gnome_index1].angle[1] < -360.0f ? 0 : buffer[gnome_index1].angle[1];
    buffer[gnome_index1].angle[0] = buffer[gnome_index1].angle[0] > 360.0f ? 0 : buffer[gnome_index1].angle[0];
    buffer[gnome_index1].angle[0] = buffer[gnome_index1].angle[0] < -360.0f ? 0 : buffer[gnome_index1].angle[0];

    buffer[gnome_index1].angle[1] += delta_time * 1000;
    buffer[gnome_index1].angle[0] += delta_time * 1000;


    static int stretch = 1;
    if(buffer[gnome_index3].scale[2] < 5 && stretch) {buffer[gnome_index3].scale[2]+=delta_time; stretch = 1;}
    else if(buffer[gnome_index3].scale[2] > 1) {buffer[gnome_index3].scale[2]-=delta_time; stretch = 0;}
    else stretch = 1;
}

static void menu_update(AppData* data)
{

    physic(data->delta);
}

static void menu_input(AppData* data, SDL_Event *e)
{
    switch (e->type) {
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_TEXTINPUT:
            nk_buffer_event_push(data, e);
            break;
        default:
            break; // Ignore window events, sensor events, etc.
    } 

}
static void menu_render(AppData* data)
{
    //Because nuklear is immediate mode gui it needs to be called at the same frequency the window inputs are processed
    //this kind of ruins my decoupling of input from the rendering and now whenever we are in the MENU state the handle_input does not handle
    //input but passes it through the buffer to this rendering function where it's actually processed, fucking wonderful

    nk_input_begin(data->ctx); //It sets some internal states in the nuklear, needs to be called right before event poll
    for (int i = 0; i < data->nk_event_buffer.count; i++)
    {
        nk_sdl_handle_event(&data->nk_event_buffer.events[i]); // let nuclear process inputs first
    }
    nk_sdl_handle_grab(); //Handles grabbable menus, which i will propably use 
    nk_input_end(data->ctx); //also sets some states but after the input is processed
    data->nk_event_buffer.count = 0; // reset to 0 so the previous inputs don't need to be processed

    struct nk_context* ctx = data->ctx;
    struct RendererContext* rd_ctx = &data->rd_ctx;

    /*
    struct nk_rect new_pos = nk_rect(0, 0, data->w, data->h);
    nk_window_set_bounds(ctx, "App options", new_pos);*/

    //Least efficient way to darken the screen ever
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

    if (nk_begin(ctx, "App options", nk_rect(data->w - 250, 50, 200, 200),
    NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
    NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        nk_bool active = (data->flags & APPSTATEFLAG_VSYNC_ON) ? 1 : 0;
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_checkbox_label(ctx, "Vsync", &active))
        {
            if(data->flags & APPSTATEFLAG_VSYNC_ON)
            {
                if(SDL_GL_SetSwapInterval(0) < 0)
                {
                    printf("Couldn't turn off vsync? How the hell did that even happen\n");
                }
                data->flags &= ~APPSTATEFLAG_VSYNC_ON;
                data->render_accumulator = 0; // nothing to accumulate frames uncapped
            } 
            else
            {
                if(SDL_GL_SetSwapInterval(-1) < 0)
                {
                    printf("Adaptive VSync not supported\n");
                    if (SDL_GL_SetSwapInterval(1) < 0) 
                    {
                        printf("VSync not supported: %s\n", SDL_GetError());
                    }
                }
                data->flags |= APPSTATEFLAG_VSYNC_ON;
                data->render_accumulator = SDL_GetPerformanceFrequency() / (data->refresh_rate + 5);
            }
        }
    }
    nk_end(ctx);

    //This is ripped from the example nuklear sdl program for testing
    if (nk_begin(ctx, "Renderer elements", nk_rect(50, 50, 300, 300),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        struct nk_colorf bg;
        bg.r = rd_ctx->main_light->color[0];
        bg.g = rd_ctx->main_light->color[1];
        bg.b = rd_ctx->main_light->color[2];
    
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Light:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
            nk_layout_row_dynamic(ctx, 120, 1);
            bg = nk_color_picker(ctx, bg, NK_RGB);
            nk_layout_row_dynamic(ctx, 25, 1);
            bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
            bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
            bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);\
            //update the actual light color of the renderer
            light_change_color(rd_ctx->main_light, bg.r, bg.g, bg.b);
            nk_combo_end(ctx);
        }

        /*Start of the Model settings*/
    
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Models:", NK_TEXT_LEFT);
        static int selected_idx; //remembers previous choice
        static int selection_valid; //a flag
        int model_count = rm_get_named_model_count(); // the amount of menu options
        ModelHashMap* m_hash = rm_get_named_model_hashmap();
        char* chosen_model_name;
        if(model_count > selected_idx)
        {
            chosen_model_name = m_hash[selected_idx].key;
            selection_valid = 1;
        }
        else
        {
            chosen_model_name = "INVALID MODEL";
            selection_valid = 0;
        }
    
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_label(ctx, chosen_model_name, nk_vec2(nk_widget_width(ctx), 200))) 
        {
            int ROW_HEIGHT = 25;
            struct nk_list_view view;
            nk_layout_row_dynamic(ctx, ROW_HEIGHT*9, 1); // this is the height of the dropdown list
            if (nk_list_view_begin(ctx, &view, "Model List", NK_WINDOW_BORDER, ROW_HEIGHT, model_count)) //this thing makes it so only elements which are visible are considered
            {
                nk_layout_row_dynamic(ctx, ROW_HEIGHT, 1); 
                for (int i = view.begin; i < view.end; i++) {
                    nk_bool selected = (selected_idx == i);
                    if (nk_selectable_label(ctx, m_hash[i].key, NK_TEXT_LEFT, &selected)) {
                        selected_idx = i;
                        nk_combo_close(ctx); // Close the dropdown once selected
                    }
                }
                nk_list_view_end(&view);
            }
            nk_combo_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, nk_rgb(100, 100, 100), nk_true);

        if(selection_valid)
        {
            nk_layout_row_dynamic(ctx, 300, 1);
            if (nk_group_begin(ctx, "Model properties", NK_WINDOW_BORDER )) 
            {

                Model* m = &(rm_get_model_buffer()[m_hash[selected_idx].value]);

                //reuse this template
                nk_layout_row_template_begin(ctx, 25);
                nk_layout_row_template_push_static(ctx,80); 
                nk_layout_row_template_push_dynamic(ctx);  
                nk_layout_row_template_push_static(ctx,50); 
                nk_layout_row_template_end(ctx);

                nk_label(ctx, "Position X:", NK_TEXT_LEFT);
                nk_slider_float(ctx, -100.0f, &m->pos[0], 100.0f, 0.1f);
                nk_labelf(ctx, NK_TEXT_RIGHT, "%.2f", m->pos[0]);

                nk_label(ctx, "Position Y:", NK_TEXT_LEFT);
                nk_slider_float(ctx, -100.0f, &m->pos[1], 100.0f, 0.1f);
                nk_labelf(ctx, NK_TEXT_RIGHT, "%.2f", m->pos[1]);

                nk_label(ctx, "Position Z:", NK_TEXT_LEFT);
                nk_slider_float(ctx, -100.0f, &m->pos[2], 100.0f, 0.1f);
                nk_labelf(ctx, NK_TEXT_RIGHT, "%.2f", m->pos[2]);

                nk_layout_row_dynamic(ctx, 1, 1);
                nk_rule_horizontal(ctx, nk_rgb(100, 100, 100), nk_true);
                nk_layout_row_template_begin(ctx, 25);
                nk_layout_row_template_push_static(ctx, 110); 
                nk_layout_row_template_push_dynamic(ctx);  
                nk_layout_row_template_push_static(ctx,50); 
                nk_layout_row_template_end(ctx);

                nk_label(ctx, "Angle around X:", NK_TEXT_LEFT);
                nk_slider_float(ctx, -360.0f, &m->angle[0], 360.0f, 0.1f);
                nk_labelf(ctx, NK_TEXT_RIGHT, "%.2f", m->angle[0]);

                nk_label(ctx, "Angle around Y:", NK_TEXT_LEFT);
                nk_slider_float(ctx, -360.0f, &m->angle[1], 360.0f, 0.1f);
                nk_labelf(ctx, NK_TEXT_RIGHT, "%.2f", m->angle[1]);

                nk_label(ctx, "Angle around Z:", NK_TEXT_LEFT);
                nk_slider_float(ctx, -360.0f, &m->angle[2], 360.0f, 0.1f);
                nk_labelf(ctx, NK_TEXT_RIGHT, "%.2f", m->angle[2]);

                nk_layout_row_dynamic(ctx, 1, 1);
                nk_rule_horizontal(ctx, nk_rgb(100, 100, 100), nk_true);
                nk_layout_row_template_begin(ctx, 25);
                nk_layout_row_template_push_static(ctx,80); 
                nk_layout_row_template_push_dynamic(ctx);  
                nk_layout_row_template_push_static(ctx,50); 
                nk_layout_row_template_end(ctx);
                
                float max_size = fmax(fmax(m->scale[0], m->scale[1]), m->scale[2]);
                float size = max_size;
                nk_label(ctx, "Size:", NK_TEXT_LEFT);
                nk_slider_float(ctx, 0.1f, &size, 100.0f, 0.1f);
                nk_labelf(ctx, NK_TEXT_RIGHT, "%.2f", size);
                float scaling =  size / max_size;
                m->scale[0] *= scaling;
                m->scale[1] *= scaling;
                m->scale[2] *= scaling;

                /*nk_layout_row_dynamic(ctx, 25, 2);
                nk_label(ctx, "Position:", NK_TEXT_LEFT);
                m->pos[0] = nk_propertyf(ctx, "#X", -100, m->pos[0], 100, 0.1f, 0.05f);*/
                

                nk_group_end(ctx);
            }
        }
        /*End of the Model settings*/
        
    }
    nk_end(ctx);

    renderer_render(data->delta); //main rendering done here
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY); //menu rendered here
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

//wrapper, idk if there is ever gonna be more here
static void playing_render(AppData* data)
{
    renderer_render(data->delta);
}

static void playing_update(AppData* data)
{
    if(data->k_state[SDL_SCANCODE_W]) renderer_camera_move_delta_forward(data->delta);
    if(data->k_state[SDL_SCANCODE_A]) renderer_camera_move_delta_left(data->delta);
    if(data->k_state[SDL_SCANCODE_S]) renderer_camera_move_delta_back(data->delta);
    if(data->k_state[SDL_SCANCODE_D]) renderer_camera_move_delta_right(data->delta);
    if(data->k_state[SDL_SCANCODE_LSHIFT] || data->k_state[SDL_SCANCODE_RSHIFT]) renderer_camera_move_delta_down(data->delta);
    if(data->k_state[SDL_SCANCODE_SPACE]) renderer_camera_move_delta_up(data->delta);

    physic(data->delta);
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
        .render = playing_render,
        .update = playing_update,
        .handle_input = playing_input,
        .exit = dummy_func
    },
    [APPSTATE_MENU] = {
        .enter = menu_on_enter,
        .render = menu_render,
        .update = menu_update,
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

