#pragma once
#include "3d_engine/graphic_types.h"

#define DEFAULT_TEXTURE "default"

typedef enum DefaultMeshes
{
    MESH_INVALID,
    MESH_CUBE

} DefaultMeshes;

typedef enum DefaultModels
{
    MODEL_INVALID
} DefaultModels;

// some macros to specify the size of preallocation for buffers 
#define MESH_BUFFER_SIZE 256
#define MODEL_BUFFER_SIZE 64

void rm_init();
void rm_clean();

//registers texture in hashmap
void rm_reg_texture(char* path, Texture tex);
void rm_del_texture(char *path);
Texture rm_get_texture(char *path);

unsigned rm_add_mesh(Mesh mesh);
unsigned rm_get_mesh_count();
// an alternative to making the buffer extern? idk
Mesh* rm_get_mesh_buffer();

Model* rm_next_named_model(char* name);
unsigned rm_copy_named_model(char* name, char* name_dest);
unsigned rm_get_named_model_index(char* name);
unsigned rm_add_named_model(char* name, Model model);
unsigned rm_add_model(Model model);
unsigned rm_get_model_count();
Model* rm_get_model_buffer();