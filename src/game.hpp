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
		void update_particles();

		void shoot();

		LightsData lights;
		Material rail_material;
		Texture2D * rail_texture;
		Path * path;
		Rails * rails;
		Player player;
		ParticleSystem * attack_particles, *dust, *smoke;
		particle_config_t particle_types[3];
		particle_type_t current_particle_type;
		int smoke_count;
		float smoke_spawn_speed;

		TextureArray * particle_textures;

		Terrain * terrain;

		Input input;

		Camera camera;
		RenderTarget *composition;

		Shader *particle_shader;

		glm::vec4 wind_velocity;
		glm::vec4 gravity;

		glm::vec3 camera_offset;
		float look_at_offset;
		float movement_speed;
		float brake_movement_speed;

		float current_movement_speed;

		Color sky_color;

};

#endif
