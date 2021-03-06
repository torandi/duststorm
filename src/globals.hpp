#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef HAVE_CONFIG_H
       #include "config.h"
#endif

#include "platform.h"

#include <cstdio>

#include "shader.hpp"
#include "cl.hpp"
#include <glm/glm.hpp>

#ifdef WIN32
#	include "wii.hpp"
	extern wii* WII;						/* Wiimote */
#endif

extern FILE* verbose;                    /* stderr if verbose output is enabled or /dev/null if not */
extern float global_time;                 /* current time */
extern glm::ivec2 resolution;            /* current resolution */
extern glm::mat4 screen_ortho;           /* orthographic projection for window */
extern CL * opencl;
extern bool useWII;						/* Whether we should use the Wiimote. */
extern bool music_mute;

enum shader_t {
	SHADER_SIMPLE=0,
	SHADER_NORMAL,
	SHADER_DEBUG,
	SHADER_SKYBOX,
	SHADER_WATER,
	SHADER_PARTICLES,

	/* for rendering targets */
	SHADER_PASSTHRU,                       /* multiplies vertices with MVP and textures using unit 1 */
	SHADER_BLUR,                           /* gaussian blur */
	SHADER_BLEND,                          /* mixes texture unit 1-4 using unit 5 */

	NUM_SHADERS
};

extern Shader* shaders[];                /* all shader programs */

#endif /* GLOBALS_H */
