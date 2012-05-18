#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include "shader.hpp"

/**
 * Current resolution
 */
extern glm::ivec2 resolution;

/**
 * orthographic projection for primary fbo.
 */
extern glm::mat4 screen_ortho;

extern Shader * shader;

/**
 * Get the current in-engine time.
 */
float get_time();

int checkForGLErrors( const char *s );

#endif
