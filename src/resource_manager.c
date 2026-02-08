// Only one instance of a resource manager can exist, which is not really a problem for this project
// A lot of functions here are just wrappers for stb_ds library
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h> // mainly for hash map implementation

#include "3d_engine/graphic_types.h"
#include "3d_engine/resource_manager.h"
#include "3d_engine/texture.h"
#include "3d_engine/renderer.h"

//this maps string values of filenames (and any specified name) to Texture struct NOTE: idk if the Texture struct is needed it would work if it was mapping to the GLUINT id
static TextureHashMap* texture_hash_map = NULL;

//this points to indices in model buffer
static ModelHashMap* model_hash_map = NULL;
//dynamic array
static Model* model_buffer = NULL;

//dynamic array
//TODO: Maybe each model should just have the mesh buffer, it would definetely make freeing it infinitely easier. Or maybe the mesh buffer could just contain some primitives like cubes that never need to be freed?
static Mesh* mesh_buffer = NULL;

// registers texture in hashmap
void rm_reg_texture(char *path, Texture tex)
{
    stbds_shput(texture_hash_map, path, tex);
}

void rm_del_texture(char *path)
{
    //TODO: the texture needs to be freed from the gpu first but that's a different can of worms
    stbds_shdel(texture_hash_map, path);
}

Texture rm_get_texture(char *path)
{
    return stbds_shget(texture_hash_map, path);
}

//adds the mesh to dynamic buffer and returns the index
unsigned rm_add_mesh(Mesh mesh)
{
    arrpush(mesh_buffer, mesh);
    return (unsigned)(arrlen(mesh_buffer) - 1);
}

unsigned rm_get_mesh_count() {
    return (unsigned)arrlen(mesh_buffer);
}
// an alternative to making the buffer extern? idk
Mesh* rm_get_mesh_buffer() {
    return mesh_buffer;
}


//Like add named model except that it just extends the array and returns the pointer to the element to write to (do not keep it)
Model* rm_next_named_model(char* name)
{
    if (shgeti(model_hash_map, name) == -1)
    {
        unsigned i = arraddnindex(model_buffer, 1);
        shput(model_hash_map, name, i);
        return &model_buffer[i];
    }
}
unsigned rm_copy_named_model(char* name, char* name_dest)
{
    unsigned i = rm_get_named_model_index(name);
    const Model* new_model = rm_next_named_model(name_dest);
    memcpy(new_model, &model_buffer[i], sizeof(Model));
    return i;
}

unsigned rm_get_named_model_index(char* name)
{
    return shget(model_hash_map, name);   
}

//basically the rm_add_model but also adds name to the hashmap
unsigned rm_add_named_model(char* name, Model model)
{
    arrpush(model_buffer, model);
    unsigned i = (unsigned)(arrlen(model_buffer) - 1);
    shput(model_hash_map, name, i);
}

unsigned rm_get_named_model_count()
{
    return shlenu(model_hash_map);
}

ModelHashMap* rm_get_named_model_hashmap()
{
    return model_hash_map;
}

unsigned rm_add_model(Model model)
{
    arrpush(model_buffer, model);
    return (unsigned)(arrlenu(model_buffer) - 1);
}

unsigned rm_get_model_count() {
    return (unsigned)arrlenu(model_buffer);
}
// an alternative to making the buffer extern? idk
Model* rm_get_model_buffer() {
    return model_buffer;
}

//some stuff that maybe we want to be inside the buffers at all times
void rm_init()
{
    arrsetcap(model_buffer, MODEL_BUFFER_SIZE);
    arrsetcap(mesh_buffer, MESH_BUFFER_SIZE);

    //first buffer element is gonna be pointing to nothing
    arrpush(model_buffer, (Model){0});
    arrpush(mesh_buffer, (Mesh){0});

    //I really don't like this being here 
    Mesh mesh;
    unsigned i = rm_add_mesh(mesh);
    mesh_create_primitive_cube(&mesh_buffer[i]);

    Texture tex = texture_create_color(DEFAULT_COLOR);
    rm_reg_texture("default", tex); 
}

void rm_clean()
{
    shfree(texture_hash_map);
    shfree(model_hash_map);
    arrfree(mesh_buffer);
    arrfree(model_buffer);
}