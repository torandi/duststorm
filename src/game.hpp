#ifndef GAME_CPP
#define GAME_CPP

#include <SDL/SDL.h>
#include "globals.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"
#include "render_object.hpp"
#include "shader.hpp"
#include "quad.hpp"
#include "terrain.hpp"
#include "lights_data.hpp"

#include "input.hpp"

#include "sound.hpp"

#include <list>
#include <map>
#include <string>

class Game {
	public:
		Game();
		~Game();

		void update(float dt);

		void handle_input(const SDL_Event &event);

		void render();

		static void init();

		Sound * play_sfx(const std::string &str, float delay= -1.f, int loops = 0);
		Sound * play_sfx_nolist(const std::string &str, float delay= -1.f, int loops = 0);

	private:
		GLint u_texture_mix;
		Shader * mix_shader;
		RenderObject * model;
		LightsData lights;


		Input input;
		void render_composition();
		void render_display();

		void render_content();
		void render_statics();
		void render_dynamics();

		void do_action(int num);

		Camera camera;
		RenderTarget *screen, *composition;//, *downsample[2];

		std::map<std::string, std::string> sfx;
		std::list<Sound*> active_sfx;

		//Shader * dof_shader;
	
		glm::vec3 camera_offset;
};

#endif
