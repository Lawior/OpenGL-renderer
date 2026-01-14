//This is for loading shaders from files, compiling them and running programs
//Here there should be the error checking for linking and compilation if i have time for it. Otherwise it may as well be in the render.c file
#include <stdio.h>
#include <stdlib.h>

#include "glad/glad.h"

typedef struct Uniforms
{

} Uniforms;

//helper function for logging
static const char* get_shader_name(GLint type) {
    switch (type) {
        case GL_VERTEX_SHADER:   return "VERTEX";
        case GL_FRAGMENT_SHADER: return "FRAGMENT";
        case GL_GEOMETRY_SHADER: return "GEOMETRY";
        default: return "UNKNOWN";
    }
}

//REMINDER: This returns a dynamic buffer it needs to be freed
static char* shader_from_file(const char* shader_path, GLint shader_type)
{
    const char* folder = "shaders/";

    // should never be exceeded
    char full_shader_path[512];
    snprintf(full_shader_path, sizeof(full_shader_path), "%s%s", folder, shader_path);

    // using the rb so it doesn't fuck up on the end of line characters
    FILE* f = fopen(full_shader_path,"rb");
    char* buffer;
    if(f == NULL)
    {
        printf("Couldn't find %s SHADER FILE AT %s\n", get_shader_name(shader_type), full_shader_path);
        fclose(f);
        return NULL; 
    }
    fseek(f, 0, SEEK_END);
    long s = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = (char*)malloc(sizeof(char)*s + 1 );

    size_t size_read = fread(buffer, sizeof(char), s, f);
    buffer[size_read] = '\0';
    
    fclose(f);
    return buffer;
}

static GLuint compile_shader(const char* shader_path, GLint shader_type)
{
    char* shader_code = shader_from_file(shader_path, shader_type);
    if(shader_code == NULL) return 0;

    GLuint shader = glCreateShader(shader_type); 
    glShaderSource(shader, 1, (const char**)&shader_code, NULL);   
   
    glCompileShader(shader);

    //Error logging for compilation
    int comp_success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &comp_success);
    free(shader_code);
    if (comp_success == GL_FALSE) {
        GLint log_size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        GLchar info_log[log_size];

        glGetShaderInfoLog(shader, log_size, NULL, info_log);
        //TODO: This is an error message format from the Learnopengl, maybe i should make it different
        //maybe think of some logging system or something, stdout may not be the best
        printf("ERROR::SHADER::%s::COMPILATION_FAILED\n%s\n", get_shader_name(shader_type), info_log);
        
        glDeleteShader(shader);
        return 0;
    }
    else return shader;
}

// this is the outside
GLuint create_shader_program(const char* vertex_path, const char* fragment_path)
{
    GLuint vertex_shader = compile_shader(vertex_path, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_path, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    // regardless of the outcome delete the shader it won't be needed
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    //error logging for program linking
    int link_success;
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if (link_success == GL_FALSE) {
        GLint log_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        GLchar info_log[log_size];

        glGetProgramInfoLog(program, log_size, NULL, info_log);
        printf("FAILED TO LINK THE SHADER PROGRAM\n%s\n", info_log);

        glDeleteProgram(program);
        return 0;
    }
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    return program;

}
