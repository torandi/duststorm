#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "particle_system.hpp"

#define NUM_PARTICLES 1000

class ParticlesScene: public Scene {
public:
	ParticlesScene(const glm::ivec2 &size)
		: Scene(size)
		, particles(NUM_PARTICLES, Texture::array(3, "textures/fire1.png", "textures/fire2.png", "textures/fire3.png"))
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.f) {
			camera.set_position(glm::vec3(0.f, 0.f, -1));
			camera.look_at(glm::vec3(0.f, 0.f, 0.f));
			particles.config.spawn_position = glm::vec4(0.f, -0.5f, -0.2f, 1.f);
			particles.config.spawn_area = glm::vec4(0.4f, 0.f, 0.4f, 0.f);
			particles.config.spawn_direction = glm::vec4(0, 1.f, 0.f, 0.f);
			particles.config.direction_var = glm::vec4(0.1f, 0.f, 0.1f, 0.f);
			particles.config.avg_spawn_speed= 0.001f;
			particles.config.spawn_speed_var = 0.0005f;
			particles.config.avg_ttl = 7.f;
			particles.config.ttl_var = 2.5f;
			particles.config.avg_spawn_delay = 0.f;
			particles.config.spawn_delay_var = 0.f;
			particles.config.avg_scale = 0.4f;
			particles.config.scale_var = 0.05f;
			particles.config.avg_scale_change = 0.2f;
			particles.config.scale_change_var = 0.05f;
			particles.config.avg_rotation_speed = 0.01f;
			particles.config.rotation_speed_var = 0.01f;
			particles.config.birth_color = glm::vec4(1.0, 0.8, 0.0, 1.0);
			particles.config.death_color = glm::vec4(0.8 ,0.2, 0.1, 0.f);
			particles.config.motion_rand = glm::vec4(0.0, 0.f, 0.0, 0);
			particles.update_config();
/*
			Light l(Light::POINT_LIGHT, glm::vec3(0.8, 0.8, 0.8));
			l.set_half_light_distance(0.25f);

			l.set_position(glm::vec3(0, 0, -1));*/
			lights.num_lights = 0;
			lights.ambient_intensity = glm::vec3(1.0, 1.0, 1.0);
			//lights.lights[0] = l.shader_light();
	}

	virtual void render(){
		clear(Color::black);
		Shader::upload_camera(camera);

		Shader::upload_lights(lights);

		shaders[SHADER_PARTICLES]->bind();
		{
			Shader::upload_model_matrix(glm::mat4(1.f));
			particles.render();
		}
		shaders[SHADER_PARTICLES]->unbind();

	}

	virtual void update(float t, float dt){
		particles.update(dt);
	}

private:
	ParticleSystem particles;
	Camera camera;
	Shader::lights_data_t lights;
};

REGISTER_SCENE_TYPE(ParticlesScene, "Particles");
