#ifndef GLOBALS_H
#define GLOBALS_H

#include "shader.hpp"
#include "time.hpp"
#include <glm/glm.hpp>

#include "cl.hpp"

extern Time global_time;                 /* current time */
extern glm::ivec2 resolution;            /* current resolution */
extern glm::mat4 screen_ortho;           /* orthographic projection for primary fbo */

enum shader_t {
	SHADER_SIMPLE=0,
	SHADER_NORMAL,
   SHADER_PARTICLES,
   SHADER_DEBUG,
	NUM_SHADERS
};

extern Shader* shaders[];                /* all shader programs */
extern CL * opencl;

#endif /* GLOBALS_H */
