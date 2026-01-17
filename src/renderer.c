#include <stdio.h>
#include <cglm/cglm.h>
#include <time.h>
#include <math.h>

#include "glad/glad.h"
#include "3d_engine/renderer.h"
#include "3d_engine/shader.h"
#include "3d_engine/texture.h"

typedef struct Mesh{
    GLuint VAO;
    GLuint EBO;
    GLuint VBO;
    unsigned EBO_size;
} Mesh;

typedef struct Object
{
    vec3 pos;
    vec3 angle;
    Mesh mesh;
    mat4 model_matrix; //this is the rotation scale and position int the global coordinates
} Object;

typedef struct Uniforms{
    GLuint model;
    GLuint view;
    GLuint projection;
    GLuint color;   
} Uniforms;

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

// this global is here so that the renderer doesn't need to query the opengl for viewport every frame but only when it's resized
static vec2 main_viewport;

static Object cube;
//basic shader program for now we don't need more
static GLuint main_program;
// this is the main camera, it is used in quite a few functions which are called from outside the file in fact all renderer_ for the 
// camer use this
static Camera main_camera; 

static Texture tex1;

Uniforms main_uniforms;

//this sets default variables for the renderer in the opengl state machine
//it needs to be called every loop because of different library also managing the state
static void set_default_opengl()
{
    //glDisable(GL_SCISSOR_TEST); 
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glDisable(GL_BLEND);
}

static void update_model_matrix(Object* obj)
{
    glm_mat4_identity(obj->model_matrix);
    glm_translate(obj->model_matrix, obj->pos);
    glm_rotate(obj->model_matrix, obj->angle[0], (vec3){1,0,0});
    glm_rotate(obj->model_matrix, obj->angle[1], (vec3){0,1,0});
    glm_rotate(obj->model_matrix, obj->angle[2], (vec3){0,0,1});
}

static void create_cube_object(Object* cube, vec3 pos, vec3 angle)
{
float vertices[] = {
    // Front face
    -1.0f, -1.0f,  1.0f,  0.0f, 0.0f, // Bottom-left
     1.0f, -1.0f,  1.0f,  1.0f, 0.0f, // Bottom-right
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, // Top-right
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f, // Top-left

    // Back face
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, // Bottom-left (mirrored for back)
     1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 
     1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 
    -1.0f,  1.0f, -1.0f,  1.0f, 1.0f,

    // Left face
    -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, // Top-right
    -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, // Top-left
    -1.0f, -1.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, // Bottom-right

    // Right face
     1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 0.0f,

    // Bottom face
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,

    // Top face
    -1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f
    };
unsigned indices[] = {
    // Front face (Vertices 0, 1, 2, 3)
    0, 1, 2,  2, 3, 0,
    
    // Back face (Vertices 4, 5, 6, 7)
    4, 5, 6,  6, 7, 4,
    
    // Left face (Vertices 8, 9, 10, 11)
    8, 9, 10,  10, 11, 8,
    
    // Right face (Vertices 12, 13, 14, 15)
    12, 13, 14,  14, 15, 12,
    
    // Bottom face (Vertices 16, 17, 18, 19)
    16, 17, 18,  18, 19, 16,
    
    // Top face (Vertices 20, 21, 22, 23)
    20, 21, 22,  22, 23, 20
};

    glGenVertexArrays(1, &cube->mesh.VAO);
    glBindVertexArray(cube->mesh.VAO);

    glGenBuffers(1, &cube->mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube->mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &cube->mesh.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    cube->mesh.EBO_size = sizeof(indices)/sizeof(unsigned);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1); 
    
    glm_vec3_copy(pos, cube->pos);
    glm_vec3_copy(angle, cube->angle);
    //i guess it's here to make sure  it's not empty
    glm_mat4_identity(cube->model_matrix);
}

static void draw_object(Object* obj)
{
    // update the model matrix from your own parameters before sending it off to gpu
    update_model_matrix(obj);

    glBindVertexArray(obj->mesh.VAO);
    glUniformMatrix4fv(main_uniforms.model, 1, GL_FALSE, (float*)obj->model_matrix);
    glUniform4f(main_uniforms.color, 1.0, 0, 0 , 1.0);
    glBindTexture(GL_TEXTURE_2D, tex1.id); 
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //for wireframe
    glDrawElements(GL_TRIANGLES, obj->mesh.EBO_size, GL_UNSIGNED_INT, 0);
}

static inline void camera_init(Camera* camera, int fov, vec3 pos, 
    float nearZ, float farZ,
    float movement_speed,
    float pitch, float yaw)
{
    camera->fov = fov;
    camera->nearZ = nearZ;
    camera->farZ = farZ;
    glm_vec3_copy(pos, camera->pos);
    camera->movement_speed = movement_speed;
    camera->yaw = yaw;
    camera->pitch = pitch;

    //Magic Euler angles idk how they work but they do
    camera->direction[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    camera->direction[1] = sin(glm_rad(camera->pitch));
    camera->direction[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
}

//this function actually does an update on the camera matrices from the Camera struct own variables
//It also immediately updates asociated uniforms
static void camera_update_matrices(Camera* camera, const Uniforms* uni)
{
    glm_perspective(glm_rad(camera->fov), 
    camera->aspect,
    camera->nearZ,
    camera->farZ,
    camera->projection_matrix);

    /* below attempt at my own version of glm_lookat
    //substract position from target getting a vector
    //also in fact it's not a direction vector because it's facing towards the camera (because -z is forward in opengl)
    vec3 direction;
    // _sub is vector substraction _subs would be scalar
    glm_vec3_sub(camera->pos, camera->direction, direction);
    glm_vec3_normalize(direction);

    // calculating the vector that will server as x axis in the camera coordinate space. The direction is the z axis
    vec3 up = {0,1,0};
    vec3 camera_x_axis;
    glm_vec3_cross(up, direction, camera_x_axis)
    glm_vec3_normalize(camera_x_axis);

    vec3 camera_y_axis;
    glm_vec3_cross(camera_x_axis, direction, camera_y_axis);
    */
    vec3 target;
    glm_vec3_add(camera->pos, camera->direction, target);
    glm_lookat(camera->pos, target, (vec3){0,1,0}, camera->view_matrix); 

    glUniformMatrix4fv(uni->view, 1, GL_FALSE, (float*)camera->view_matrix);
    glUniformMatrix4fv(uni->projection, 1, GL_FALSE, (float*)camera->projection_matrix);
}

//below functions renderer_camera_move_delta are functions that move in specific direction based on delta time passed
//move camera in the direction of you are facing
void renderer_camera_move_delta_forward(double delta_time)
{
    float movement = main_camera.movement_speed * delta_time; //I wonder if precision is ok in this
    vec3 direction;
    glm_vec3_scale(main_camera.direction, movement, direction);
    glm_vec3_add(main_camera.pos, direction, main_camera.pos);
}

//move camera in the direction away from the dircetion you are facing
void renderer_camera_move_delta_back(double delta_time)
{
    float movement = main_camera.movement_speed * delta_time; 
    vec3 direction;
    glm_vec3_scale(main_camera.direction, 0-movement, direction);
    glm_vec3_add(main_camera.pos, direction, main_camera.pos);
}
void renderer_camera_move_delta_right(double delta_time)
{
    float movement = main_camera.movement_speed * delta_time; 
    vec3 up = {0,1,0};
    vec3 direction;
    glm_vec3_cross(main_camera.direction, up, direction);
    glm_vec3_normalize(direction);
    glm_vec3_scale(direction, movement, direction);
    glm_vec3_add(main_camera.pos, direction, main_camera.pos);
}

void renderer_camera_move_delta_left(double delta_time)
{
    float movement = main_camera.movement_speed * delta_time; 
    vec3 up = {0,1,0};
    vec3 direction;
    glm_vec3_cross(up, main_camera.direction, direction);
    glm_vec3_normalize(direction);
    glm_vec3_scale(direction, movement, direction);
    glm_vec3_add(main_camera.pos, direction, main_camera.pos);
}
 
void renderer_camera_move_delta_down(double delta_time)
{
    float movement = main_camera.movement_speed * delta_time; 
    vec3 down = {0,-1,0};
    glm_vec3_scale(down, movement, down);
    glm_vec3_add(main_camera.pos, down, main_camera.pos);
}

void renderer_camera_move_delta_up(double delta_time)
{
    float movement = main_camera.movement_speed * delta_time; 
    vec3 up = {0,1,0};
    glm_vec3_scale(up, movement, up);
    glm_vec3_add(main_camera.pos, up, main_camera.pos);
}



void renderer_camera_rotate(int x_rel, int y_rel)
{
    float sensitivity = 0.1; // this should be moved somewhere else later
    
    main_camera.yaw += x_rel*sensitivity;
    main_camera.pitch -= y_rel*sensitivity; //inverse because screen coordinates

    //The math for the view matrix depends on this value not being >= 90
    if(main_camera.pitch > 89.0) main_camera.pitch = 89.0;
    else if (main_camera.pitch < -89.0) main_camera.pitch = -89.0;

    if (main_camera.yaw > 360.0f)  main_camera.yaw -= 360.0f;
    else if (main_camera.yaw < -360.0f) main_camera.yaw += 360.0f;

    main_camera.direction[0] = cos(glm_rad(main_camera.yaw)) * cos(glm_rad(main_camera.pitch));
    main_camera.direction[1] = sin(glm_rad(main_camera.pitch));
    main_camera.direction[2] = sin(glm_rad(main_camera.yaw)) * cos(glm_rad(main_camera.pitch));
}

//this resizes the ViewPort and changes main camera aspect ratio
void renderer_resize(int w, int h)
{
    glViewport(0,0, w, h);
    main_viewport[0] = w;
    main_viewport[1] = h;
    float aspect = (float)w/(float)h;
    main_camera.aspect = aspect;
}

void renderer_init(int w, int h)
{
    camera_init(&main_camera, 90, (vec3){0,1,1},
                0.1, 100, 
                10,
                89.0, -89.0); //face forward at the start
    renderer_resize(w, h);

    main_program = create_shader_program("main.vs", "main.fs");
    main_uniforms.model = glGetUniformLocation(main_program, "model");
    main_uniforms.projection = glGetUniformLocation(main_program, "projection");
    main_uniforms.view = glGetUniformLocation(main_program, "view");
    main_uniforms.color = glGetUniformLocation(main_program, "color");

    create_cube_object(&cube, (vec3){0,0,-2}, (vec3){0,0,0});

    tex1 = create_texture("epic_texture.jpg");

    glClearColor(0.4f, 0.6f, 0.9f, 1.0f);
    
}


void renderer_render(AppData* data)
{
    set_default_opengl();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(main_program);

    camera_update_matrices(&main_camera, &main_uniforms);
    //glm_vec3_copy((vec3){(float)clock() / CLOCKS_PER_SEC,(float)clock() / CLOCKS_PER_SEC,0}, cube.angle);
    //glm_vec3_copy((vec3){(float)clock() / CLOCKS_PER_SEC,0,0}, cube.angle);
    draw_object(&cube);
}

void renderer_end()
{
    return;
}