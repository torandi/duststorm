#ifndef GAME_CPP
#define GAME_CPP

#include <SDL/SDL.h>
#include "globals.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"
#include "shader.hpp"
#include "quad.hpp"
#include "terrain.hpp"
#include "lights_data.hpp"

#include "input.hpp"

class Game {
	public:
		Game();
		~Game();

		void update(float dt);

		void handle_input(const SDL_Event &event);

		void render();
	private:

		Input input;
		void render_composition();
		void render_display();

		void render_statics();
		void render_dynamics();

		Camera camera;
		RenderTarget *screen, *composition, *downsample[3];
		LightsData lights;

		Shader * terrain_shader;
		Terrain * terrain;
		Texture2D * terrain_blendmap;
		TextureArray * terrain_textures[2];
};

#endif
