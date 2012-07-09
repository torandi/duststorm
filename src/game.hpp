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

#include "area.hpp"
#include "input.hpp"

#include <map>
#include <string>

class Game {
	public:
		Game();
		~Game();

		void update(float dt);

		void handle_input(const SDL_Event &event);

		void render();

		Area * get_area(const std::string &str) const;

		void change_area(const std::string &area, const std::string &entry_point);
	private:

		Input input;
		void render_composition();
		void render_display();

		void render_statics();
		void render_dynamics();

		void load_areas();
		void add_area(const std::string &name);

		Camera camera;
		RenderTarget *screen, *composition, *downsample[3];

		Area * current_area;
		std::map<std::string, Area*> areas;
};

#endif
