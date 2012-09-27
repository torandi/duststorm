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
#include "particle_system.hpp"
#include "hitting_particles.hpp"

#include "path.hpp"

#include "input.hpp"

#include "sound.hpp"
#include "player.hpp"
#include "text.hpp"

#include <list>
#include <map>
#include <string>

class Game {
	public:
		Game(const std::string &level, float near, float far, float fov);
		~Game();

		void update(float dt);

		void handle_input(const SDL_Event &event);

		void render();

		static void init();

		const Player &get_player() const;

		void enemy_impact(const glm::vec3 &position, bool kill = false);

	private:
		enum particle_type_t {
			LIGHT_PARTICLES = 0,
			MEDIUM_PARTICLES,
			HEAVY_PARTICLES
		};

		struct particle_config_t {
			ParticleSystem::config_t config;
			int count;
			float spawn_speed;
			float damage;
		};

		void render_display();
		void render_geometry();
		void update_camera();
		void update_enemies( float dt);

		void change_particles(particle_type_t new_type);
		void shoot();
		void evolve();
		bool start_pressed() const;

		glm::vec3 wind_velocity;
		//Call this function after changing wind_speed.
		void update_wind_velocity();

		LightsData lights;
		Material rail_material;
		Texture2D * rail_texture;
		Path * path;
		Rails * rails;
		Player player;

		ParticleSystem *dust, *smoke, *explosions;
		ParticleSystem::config_t hit_explosion, kill_explosion;
		HittingParticles * attack_particles;
		particle_config_t particle_types[3];
		particle_type_t current_particle_type;
		int smoke_count;
		int hit_explosion_count, kill_explosion_count;
		float smoke_spawn_speed;
		float dust_spawn_ahead;
		glm::vec3 half_dust_spawn_area;
		std::list<ParticleSystem::config_t*> system_configs;

		TextureArray * particle_textures;

		Terrain * terrain;

		Input input;

		Camera camera;
		RenderTarget *composition, *geometry;

		Quad *fullscreen_quad, *hud_choice_quad;
		Texture2D *hud_static_elements_tex, *game_over_texture, *hud_choice_tex, *startscreen_texture;
		void draw_selected_weap();
		
		glm::core::type::vec2 hud_lightpos, hud_mediumpos, hud_heavypos; 

		Shader *particle_shader, *passthru;

		glm::vec4 gravity;

		glm::vec3 camera_offset;
		float look_at_offset;
		float movement_speed;
		float brake_movement_speed;
		float despawn_distance;

		float current_movement_speed;

		float spawn_area_start, spawn_area_end, spawn_area_size, spawn_distance;

		float accum_unspawned;

		float player_level;
		float difficulty_increase;

		int life;

		int score;

		float last_break;

		float start_position;

		Color sky_color;

		std::list<Enemy*> enemies;

		Text life_text, score_text;
		//Stuff about sounds
		void play_sound(const char* path, int loops);
		std::list<Sound*> active_sounds;
		Sound* music;

		enum mode_t {
			MODE_READY,
			MODE_GAME,
			MODE_HIGHSCORE
		};

		mode_t current_mode;

		static const unsigned int NUM_HIGHSCORE_ENTRIES = 12;

		//Highscore stuff:
		Text highscore_entries[NUM_HIGHSCORE_ENTRIES];
		Highscore * highscore;

		void initialize(); //Also reinitialize


};

#endif
