#ifndef GLOBALS_H
#define GLOBALS_H

#include "shader.hpp"
#include "time.hpp"
#include <glm/glm.hpp>

#include "cl.hpp"

extern FILE* verbose;                    /* stderr if verbose output is enabled or /dev/null if not */
extern Time global_time;                 /* current time */
extern glm::ivec2 resolution;            /* current resolution */

enum shader_t {
	SHADER_SIMPLE=0,
	SHADER_NORMAL,
	SHADER_PARTICLES,
	SHADER_PARTICLES_LIGHT, //Particles with lighting
	SHADER_DEBUG,

	/* for rendering targets */
	SHADER_PASSTHRU,                       /* multiplies vertices with MVP and textures using unit 1 */
	SHADER_DISTORT,                        /* distorts the view by offsetting UV */
	SHADER_BLUR,                           /* gaussian blur */

	NUM_SHADERS
};

extern Shader* shaders[];                /* all shader programs */
extern CL * opencl;

#endif /* GLOBALS_H */
