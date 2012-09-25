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

		TextureArray * particle_textures;

		Terrain * terrain;

		Input input;

		Camera camera;
		RenderTarget *composition, *geometry;

		Quad *hud_static_elements;
		Texture2D *hud_static_elements_tex;

		Shader *particle_shader;

		glm::vec4 wind_velocity;
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

		Color sky_color;

		std::list<Enemy*> enemies;
};

#endif
