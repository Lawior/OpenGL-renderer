#include <stdio.h>
#include "3d_engine/graphic_types.h"

void scene_init(Scene* scene, unsigned obj_buf_size)
{
    scene->size = obj_buf_size;
    scene->obj = calloc(obj_buf_size, sizeof(Model));
    if(scene->obj == NULL)
    {
        printf("Couldn't allocate memory for object buffer");
        return;
    }
}

void scene_destroy(Scene* scene)
{
    free(scene->obj);
    scene->obj = NULL;
}

/* This will be used when we have multiple lights
void add_light(Scene* scene)
{

}
*/