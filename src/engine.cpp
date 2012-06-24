#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "engine.hpp"
#include "globals.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "utils.hpp"

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
	"blend",
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
		for(int i=0; i < NUM_SHADERS; ++i) {
			shaders[i] = Shader::create_shader(shader_programs[i]);
		}
	}

	void load_timetable(const std::string& filename){
		int ret;
		const char* tablename = filename.c_str();
		fprintf(verbose, "Loading timetable from `%s'\n", tablename);

		auto func = [](const std::string& name, float begin, float end){
			RenderTarget* target = rendertarget_by_name("scene:" + name);
			Scene* scene = nullptr;

			if ( !target ){
				fprintf(stderr, "Timetable entry for missing scene `%s', ignored.\n", name.c_str());
				return;
			} else if ( !(scene=dynamic_cast<Scene*>(target)) ){
				fprintf(stderr, "Timetable entry for RenderTarget `%s', ignored.\n", name.c_str());
				return;
			}

			scene->add_time(begin, end);
		};
		if ( (ret=timetable_parse(tablename, func)) != 0 ){
			fprintf(stderr, "Failed to read `%s': %s\n", tablename, strerror(ret));
			abort();
		}
	}

}
