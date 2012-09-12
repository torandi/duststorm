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

#include "path.hpp"

#include "input.hpp"

#include "sound.hpp"
#include "player.hpp"

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
		void render_display();
		void render_geometry();
		void update_camera();

		LightsData lights;
		Material rail_material;
		Texture2D * rail_texture;
		Path * path;
		Rails * rails;
		Player player;

		Terrain * terrain;

		Input input;

		Camera camera;
		RenderTarget *composition;

		Shader * terrain_shader;
	
		glm::vec3 camera_offset;
		float look_at_offset;
		float movement_speed;
		float brake_movement_speed;

		Color sky_color;

};

#endif
