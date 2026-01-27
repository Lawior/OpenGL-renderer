#pragma once
#include "3d_engine/graphic_types.h"

int load_static_model(const char* path,
    Model* model, //model buffer to fill some variables, here for now
    Mesh* meshes //mesh buffer to be filled
);