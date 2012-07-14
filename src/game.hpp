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
#include "player.hpp"

#include "area.hpp"
#include "input.hpp"

#include "object_template.hpp"
#include "enemy.hpp"

#include "sound.hpp"

#include <map>
#include <string>

class Game {
	public:
		Game();
		~Game();

		void update(float dt);

		void handle_input(const SDL_Event &event);

		void render();

		Area * area() const;

		Area * get_area(const std::string &str) const;

		void change_area(const std::string &area, const std::string &entry_point);

		Player * player;

		static std::map<std::string, object_template_create*> object_templates;
		static std::map<std::string, enemy_create*> enemy_creators;
		static void init();

		ObjectTemplate * create_object(const std::string &name, const YAML::Node &node, Area * a = nullptr);
		Enemy * create_enemy(const YAML::Node &node, const glm::vec2 &pos, Area * a = nullptr);

		void play_sfx(const std::string &str, float delay= -1.f, int loops = 0);
	private:


		Input input;
		void render_composition();
		void render_display();

		void render_content();
		void render_statics();
		void render_dynamics();

		void load_areas();

		void do_action(int num);

		static void dir_content(const char * dir, std::list<std::string> &files);

		void move_player();

		void update_mouse_position(int x, int y);

		void look_at_player();

		Camera camera;
		RenderTarget *screen, *composition;//, *downsample[2];

		Area * current_area;
		std::map<std::string, Area*> areas;
		std::map<std::string, std::string> sfx;
		std::list<Sound*> active_sfx;

		//Shader * dof_shader;
	
		glm::vec2 mouse_position;
		Texture2D * mouse_marker_texture;
		glm::vec3 camera_offset;

		enum {
			MOVE_UP = 0,
			MOVE_DOWN,
			MOVE_RIGHT,
			MOVE_LEFT,
			MOUSE_1,
			MOUSE_2,
			NUM_ACTIVE_ACTIONS
		};
		bool sustained_action[NUM_ACTIVE_ACTIONS];
};

#endif
