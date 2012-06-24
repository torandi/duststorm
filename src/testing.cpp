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

#include "music.hpp"

static RenderTarget* composition;
static RenderTarget* downsample[3];
static XYLerpTable* particle_pos = nullptr;
static XYLerpTable* tv_pos = nullptr;
static XYLerpTable* test_pos = nullptr;
static std::map<std::string, Scene*> scene;

static Music * music;

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
		/* Instantiate all scenes */
		scene["Test"] = SceneFactory::create("Test", glm::ivec2(resolution.x, resolution.y/3));
		scene["particle"] = SceneFactory::create("Particles", glm::ivec2(resolution.x, resolution.y));
		//scene["TV"] = SceneFactory::create("TV", glm::ivec2(resolution.x/2, 2*resolution.y/3));
		scene["TV"] = SceneFactory::create("TV", glm::ivec2(resolution.x, resolution.y));
		scene["Water"] = SceneFactory::create("Water", glm::ivec2(resolution.x, resolution.y));
		scene["NOX"] = SceneFactory::create("NördtroXy II", glm::ivec2(resolution.x, resolution.y));

		load_timetable(PATH_SRC "timetable.txt");

		particle_pos = new XYLerpTable("scene/particles_pos.txt");
		tv_pos = new XYLerpTable("scene/tv_pos.txt");
		test_pos = new XYLerpTable("scene/test_pos.txt");

		composition = new RenderTarget(resolution, GL_RGB8, false);
		downsample[0] = new RenderTarget(glm::ivec2(200, 200), GL_RGB8, false, GL_LINEAR);
		downsample[1] = new RenderTarget(glm::ivec2(100, 100), GL_RGB8, false, GL_LINEAR);
		downsample[2] = new RenderTarget(glm::ivec2( 50, 50), GL_RGB8, false, GL_LINEAR);

		music = new Music("jumping.ogg");
	}

	void start(double seek) {
		if(seek > 0.1) {
			music->seek(seek);
		}
		music->play();
	}

	void cleanup(){
		for ( std::pair<std::string,Scene*> p : scene ){
			delete p.second;
		}
		delete music;
	}

	static void render_scene(){
		for ( std::pair<std::string,Scene*> p: scene ){
			p.second->render_scene();
		}
	}

	static void downsample_tv(){
		RenderTarget* prev = scene["TV"];
		for ( int i = 0; i < 3; i++ ){
			Shader::upload_state(downsample[i]->texture_size());
			Shader::upload_projection_view_matrices(downsample[i]->ortho(), glm::mat4());
			downsample[i]->with([prev,i](){
					prev->draw(shaders[SHADER_BLUR], glm::ivec2(0,0), downsample[i]->texture_size());
				});
			prev = downsample[i];
		}
	}

	static void render_composition(){
		RenderTarget::clear(Color::black);
		const float t = global_time.get();

		Shader::upload_state(resolution);
		Shader::upload_projection_view_matrices(composition->ortho(), glm::mat4());
		glViewport(0, 0, resolution.x, resolution.y);

		/* glActiveTexture(GL_TEXTURE1);
		texture_test->bind();
		glActiveTexture(GL_TEXTURE0);

		scene["TV"]->draw(shaders[SHADER_DISTORT], glm::ivec2(400,0));*/

		//scene["Test" ]->draw(shaders[SHADER_PASSTHRU], screen_pos(test_pos->at(t), glm::vec2(scene["Test"]->texture_size())));
		//scene["Water" ]->draw(shaders[SHADER_PASSTHRU], screen_pos(glm::vec2(0,0), glm::vec2(scene["Water"]->texture_size())));
		//scene["TV" ]->draw(shaders[SHADER_PASSTHRU], screen_pos(glm::vec2(0,0), glm::vec2(scene["TV"]->texture_size())));
		//scene["TV" ]->draw(shaders[SHADER_PASSTHRU], screen_pos(tv_pos->at(t), glm::vec2(scene["TV"]->texture_size())));
		scene["particle"]->draw(shaders[SHADER_PASSTHRU], screen_pos(glm::vec2(0,0), glm::vec2(scene["particle"]->texture_size())));
	}

	static void render_display(){
		RenderTarget::clear(Color::magenta);
		Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
		composition->draw(shaders[SHADER_PASSTHRU]);
	}

	void render(){
		render_scene();
		downsample_tv();
		composition->with(render_composition);
		render_display();
	}

	void update(float t, float dt){
		for ( std::pair<std::string,Scene*> p: scene ){
			p.second->update_scene(t, dt);
		}
		printf("TIME: %lf\n", music->time());
	}
}
