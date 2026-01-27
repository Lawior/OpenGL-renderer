//This will contain all the structs that are used for graphics (so that i dont' have to throw each one in different .h files)
#pragma once

#include <glad/glad.h>
#include <cglm/cglm.h>

#define MESH_PER_MODEL 32

typedef struct Light{
    vec3 pos;
    vec3 color;
} Light;

typedef struct Mesh{
    GLuint VAO;
    GLuint EBO;
    GLuint VBO;
    unsigned EBO_size;
} Mesh;

typedef struct Model
{
    vec3 pos;
    vec3 angle;
    vec3 scale;
    mat4 model_matrix; //this is the rotation scale and position int the global coordinates
    mat3 normal_matrix; //for now i will keep it here it's basically a model matrix but transposed and inversed, idk why this math works but it does 
    struct Mesh* mesh[MESH_PER_MODEL];
    unsigned mesh_num;
    struct Material* material;
} Model;

typedef struct Material
{
    struct Shader* shader;
    GLuint texture_id; // will be 0 if no texture
    vec3 color;
} Material;

//This should contain most common uniforms in all shaders (some values may be left empty)
//TODO: If I ever need many shaders with different uniforms i should create a union of different Shader structs
typedef struct Shader{
    GLuint shader_id; //shader program id
    GLuint model;
    GLuint view;
    GLuint projection;
    GLuint normal_matrix;
    GLuint object_color;
    GLuint light_color;  
    GLuint light_pos;
} Shader;

//maybe we will add stuff like orientation, fov, position and such as additional variables for easier read
typedef struct Camera
{
    float nearZ, farZ, aspect; // this is for the 
    unsigned fov;
    float movement_speed; // in units per second (which should be m/s in our logic)
    float pitch, yaw; // rotation around x and y respectively (in degrees)
    vec3 pos; // where camera 
    vec3 direction; //direction in space this value should be always a unit vector
    mat4 projection_matrix; 
    mat4 view_matrix; 
} Camera;

typedef struct {
    GLuint id;
    int width;
    int height;
    int nr_channels;
} Texture;  

typedef struct Scene
{
    Model* obj;
    unsigned size;
    Light light; //for now single
} Scene;