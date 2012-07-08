#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "engine.hpp"
#include "globals.hpp"
#include "quad.hpp"
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

static RenderTarget* composition = nullptr;
static RenderTarget* blend = nullptr;
#define NUM_TEXT_TEXTURES 11
static Texture2D* text[NUM_TEXT_TEXTURES];
static Quad* textarea = nullptr;
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
		scene["NOX"] = SceneFactory::create("NördtroXy II", glm::ivec2(resolution.x, resolution.y));
		composition = new RenderTarget(resolution,           GL_RGB8, false);
		blend = new RenderTarget(glm::ivec2(1,1), GL_RGBA8, false);
		char filename[64];
		for(int i=0; i < NUM_TEXT_TEXTURES; ++i) {
			sprintf(filename, "nox2/text%d.png", i);
			text[i] = Texture2D::from_filename(filename);
		}
		textarea = new Quad(glm::vec2(1.0f, -1.0f), false);
		textarea->set_scale(glm::vec3(512, 256, 1));

		load_timetable(PATH_BASE "/src/nox2.txt");

		music = new Music("jumping.ogg");
	}

	void start(double seek) {
		music->play();
		if(global_time.sync_to_music(music)) {
			fprintf(verbose, "Syncinc to music!\n");
		} else {
			printf("Warning! Syncing disabled!\n");
		}
		if(seek > 0.1) {
			music->seek(seek);
		}
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

	static float scale_text_position(const float &t, const float &duration, const float &trim) {
		float s = t / duration;
		if(s > 1.f) {
			s += (s-1.f)*(s-1.f);
		}
		return (resolution.x - 512.f + trim - (resolution.x)*s);
	}

	static void render_composition(){
		RenderTarget::clear(Color::black);

		Shader::upload_state(resolution);
		Shader::upload_projection_view_matrices(composition->ortho(), glm::mat4());
		glViewport(0, 0, resolution.x, resolution.y);

		blend->texture_bind(Shader::TEXTURE_BLEND_S);
		scene["NOX"]->draw(shaders[SHADER_BLEND]);

		const float t = global_time.get();
		/*
		if( t < 3.f ) {
			textarea->set_position(glm::vec3( resolution.x/2.f - 256 , resolution.y/2.f, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[0]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();

			//TODO: FADE!
		}
		*/

		if ( t > 23.1f && t < 35.f ){
			//Tyngelvi
			textarea->set_position(glm::vec3( scale_text_position( (t - 23.1f), 7.f, 24) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[1]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 30.5f && t < 40.f) {
			//Primary compo
			textarea->set_position(glm::vec3( scale_text_position( (t - 30.5f), 9.5f, 128) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[2]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 40.0f && t < 50.f) {
			//Barbeque
			textarea->set_position(glm::vec3( scale_text_position( (t - 40.f), 10.f, 72) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[3]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 50.0f && t < 60.f) {
			//tickets at
			textarea->set_position(glm::vec3( scale_text_position( (t - 50.f), 10.f, 60) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[4]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 60.0f && t < 66.f) {
			//last years entries
			textarea->set_position(glm::vec3( scale_text_position( (t - 60.f), 6.f, 60) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[5]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 66.0f && t < 75.f) {
			//spacehorse
			textarea->set_position(glm::vec3( scale_text_position( (t - 66.f), 9.f, 60) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[6]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 75.0f && t < 85.f) {
			//omgspace
			textarea->set_position(glm::vec3( scale_text_position( (t - 75.f), 10.f, 0) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[7]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 85.0f && t < 95.f) {
			//trollgame
			textarea->set_position(glm::vec3( scale_text_position( (t - 85.f), 10.f, 60) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[8]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 95.0f && t < 102.f) {
			//code by
			textarea->set_position(glm::vec3( scale_text_position( (t - 95.f), 7.f, 60) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[9]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
		if( t > 102.0f && t < 109.f) {
			//music by
			textarea->set_position(glm::vec3( scale_text_position( (t - 102.f), 7.f, 60) , resolution.y - 250, 0));
			shaders[SHADER_PASSTHRU]->bind();
			text[10]->texture_bind(Shader::TEXTURE_2D_0);
			textarea->render();
		}
	}

	static void render_display(){
		RenderTarget::clear(Color::magenta);
		Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
		composition->draw(shaders[SHADER_PASSTHRU]);
	}

	void render(){
		render_scene();

		const float t = global_time.get();
		float s = glm::min(t / 2.5f + 0.2f, 1.0f);
		if(t > 115) {
			s = 1.f - (t - 115.f)/5.f;
		}
		blend->with([s](){ RenderTarget::clear(Color(s, 0.0f, 0.0f, 0.0f)); });
		composition->with(render_composition);

		render_display();
	}

	void update(float t, float dt){
		if(t >= 120) {
			terminate();
		}
		for ( std::pair<std::string,Scene*> p: scene ){
			p.second->update_scene(t, dt);
		}
	}
}
