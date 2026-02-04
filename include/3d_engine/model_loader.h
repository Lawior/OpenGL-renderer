#pragma once
#include "3d_engine/graphic_types.h"

int load_static_model(const char* path,
    Material* material // passing the material for now, as we don't load it from the model yet
);