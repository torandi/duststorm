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
		Game(const std::string &level);
		~Game();

		void update(float dt);

		void handle_input(const SDL_Event &event);

		void render();

		static void init();

	private:
		LightsData lights;

		Terrain * terrain;

		Input input;
		void render_composition();
		void render_display();

		void render_geometry(const Camera &cam);

		void do_action(int num);

		Camera camera;
		RenderTarget *screen, *composition, *downsample[2];

		Shader * terrain_shader;
		Shader * dof_shader;
	
		glm::vec3 camera_offset;
};

#endif
