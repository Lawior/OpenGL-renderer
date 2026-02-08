#pragma once
#include "3d_engine/graphic_types.h"

//this is the struct through which the main app communicates with the graphics part
//this could be a start of being able to create multiple renderers perhaps?
typedef struct RendererContext{
    struct Light* main_light;
    struct Camera* main_camera;
} RendererContext;

void renderer_camera_move_delta_right(double delta_time);
void renderer_camera_move_delta_left(double delta_time);
void renderer_camera_move_delta_up(double delta_time);
void renderer_camera_move_delta_down(double delta_time);
void renderer_camera_move_delta_forward(double delta_time);
void renderer_camera_move_delta_back(double delta_time);
void renderer_camera_rotate(int x_rel, int y_rel);

void light_change_pos(Light* light, float x, float y, float z);
void light_change_color(Light* light, float r, float g, float b);

void create_mesh(Mesh* mesh, float* vertices, unsigned v_size, unsigned* indices, unsigned i_size);
void mesh_create_primitive_cube(Mesh* mesh);

void create_model(Model* model, unsigned mesh_num, vec3 pos, vec3 angle, vec3 scale, unsigned* mesh_i, Material* material);
// temporary help
void model_create_named(char* name, unsigned mesh_num, vec3 pos, vec3 angle, vec3 scale, unsigned* mesh_i, Material* material);

RendererContext renderer_init(int w, int h);
void renderer_resize(int w, int h);
void renderer_render(double delta_time);
void renderer_end();

