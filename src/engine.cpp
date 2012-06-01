#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "engine.hpp"
#include "globals.hpp"
#include "shader.hpp"

static const char* shader_programs[NUM_SHADERS] = {
	"simple",
	"normal",
	"modelviewer",
	"particles",
	"particles_light",
	"debug",
	"skybox",
	"water",
	"passthru",
	"distort",
	"blur",
	"filmgrain",
};

namespace Engine {

	void setup_opengl(){
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_BLEND);
		glCullFace(GL_BACK);
		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	void load_shaders() {
		Shader::initialize();
		for(int i=0; i < NUM_SHADERS; ++i) {
			shaders[i] = Shader::create_shader(shader_programs[i]);
		}
	}

}
