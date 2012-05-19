#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include "shader.hpp"

/**
 * Get the current in-engine time.
 */
float get_time();

int checkForGLErrors( const char *s );

#endif
