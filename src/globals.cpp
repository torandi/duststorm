#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.hpp"
#include <cstdint>
#include <cstddef>

glm::ivec2 resolution(800,600);
Shader* shaders[NUM_SHADERS];
FILE* verbose = nullptr;
Config config;
