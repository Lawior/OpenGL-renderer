#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION //other libs need to be above
#include <stb/stb_image.h>
 
#include "3d_engine/graphic_types.h"

Texture texture_create_color(vec3 color)
{
   Texture tex;
   glGenTextures(1, &tex.id);
   glBindTexture(GL_TEXTURE_2D, tex.id);

   // just in case
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, color);
   tex.height = 1;
   tex.width = 1;
   tex.nr_channels = 3;
   return tex;
   glBindTexture(GL_TEXTURE_2D, 0);
}

Texture texture_create_from_file(const char* path)
{
   const char* folder = "textures/";

   // should never be exceeded
   char full_texture_path[512];
   snprintf(full_texture_path, sizeof(full_texture_path), "%s%s", folder, path);

   Texture tex;
   stbi_set_flip_vertically_on_load(true); 
   unsigned char *data = stbi_load(full_texture_path, &tex.width, &tex.height, &tex.nr_channels, 4);//force RGBA
   glGenTextures(1, &tex.id);
   glBindTexture(GL_TEXTURE_2D, tex.id); 

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Wraps texture on x coordinate, if it goes outside <0;1> range (s in texture coordinate system)
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Same as above but for y (t in texture coordinate system)
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); // we use mipmaps which are downscaled versions of the texture at long distance (bilinear filtering when changing between mipmaps and nearest when resizing)
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // stretch nearest neighbor

   if (data != NULL)
   {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
   }
   else 
   {
      printf("Failed to load an image for a texture with path %s: \n", full_texture_path, stbi_failure_reason());
   }
   stbi_image_free(data);
   glBindTexture(GL_TEXTURE_2D, 0);
   return tex;
}