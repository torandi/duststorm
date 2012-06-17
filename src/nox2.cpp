#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "engine.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "rendertarget.hpp"
#include "utils.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "time.hpp"
#include "cl.hpp"
#include "texture.hpp"
#include "timetable.hpp"

#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unistd.h>
#include <getopt.h>
#include <map>

#include "light.hpp"

static RenderTarget* composition;
static std::map<std::string, Scene*> scene;

namespace Engine {
	RenderTarget* rendertarget_by_name(const std::string& fullname){
		const size_t offset = fullname.find_first_of(":");
		if ( offset == std::string::npos ){
			return nullptr;
		}

		const std::string prefix = fullname.substr(0, offset);
		const std::string name   = fullname.substr(offset+1);

		if ( prefix == "scene" ){
			auto it = scene.find(name);
			if ( it == scene.end() ) return nullptr;
			return it->second;
		}

		return nullptr;
	}

	void init(){
		scene["NOX"] = SceneFactory::create("NördtroXy II", glm::ivec2(resolution.x, resolution.y));
		composition   = new RenderTarget(resolution,           GL_RGB8, false);

		load_timetable(PATH_SRC "nox2.txt");
	}

	void cleanup(){
		for ( std::pair<std::string,Scene*> p : scene ){
			delete p.second;
		}
	}

	static void render_scene(){
		for ( std::pair<std::string,Scene*> p: scene ){
			p.second->render_scene();
		}
	}

	static void render_composition(){
		RenderTarget::clear(Color::black);

		Shader::upload_state(resolution);
		Shader::upload_projection_view_matrices(composition->ortho(), glm::mat4());
		glViewport(0, 0, resolution.x, resolution.y);

		scene["NOX"]->draw(shaders[SHADER_PASSTHRU]);
	}

	static void render_display(){
		RenderTarget::clear(Color::magenta);
		Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
		composition->draw(shaders[SHADER_PASSTHRU]);
	}

	void render(){
		render_scene();
		composition->with(render_composition);
		render_display();
	}

	void update(float t, float dt){
		for ( std::pair<std::string,Scene*> p: scene ){
			p.second->update_scene(t, dt);
		}
	}
}