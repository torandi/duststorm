#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.hpp"
#include <cstdint>
#include <cstddef>

glm::ivec2 resolution;
glm::mat4 screen_ortho;
Shader* shaders[NUM_SHADERS];
CL * opencl;
