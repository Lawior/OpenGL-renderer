#pragma once

   typedef struct {
    unsigned int id;
    int width;
    int height;
    int nrChannels;
} Texture;  

Texture create_texture(const char* path);