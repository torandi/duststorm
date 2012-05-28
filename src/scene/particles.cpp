#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "particle_system.hpp"

class ParticlesScene: public Scene {
public:
	ParticlesScene(const glm::ivec2 &size)
		: Scene(size)
		, particles(1000000)
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.f) {
			camera.set_position(glm::vec3(0.f, 0.f, -1));
			camera.look_at(glm::vec3(0.f, 0.f, 0.f));
			particles.config.spawn_position = glm::vec4(-1.f, -0.2f, -1.f, 1.f);
			particles.config.spawn_area = glm::vec4(2.f, 0.f, 2.f, 0.f);
			particles.config.spawn_direction = glm::vec4(0, 1.f, 0.f, 0.f);
			particles.config.direction_var = glm::vec4(0.5f, 0.f, 0.5f, 0.f);
			particles.config.avg_spawn_speed= 0.001f;
			particles.config.spawn_speed_var = 0.0005f;
			particles.config.avg_scale = 0.05f;
			particles.config.scale_var = 0.005f;
			particles.config.birth_color = glm::vec4(0.8, 0.8, 0.8, 0.8);
			particles.config.death_color = glm::vec4(0.8, 0.8, 0.8, 0.1);
			particles.config.motion_rand = glm::vec4(0, 0, 0, 0);
			particles.update_config();
	}

	virtual void render(){
		clear(Color::black);
		Shader::upload_camera(camera);

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
	double rotation;
};

REGISTER_SCENE_TYPE(ParticlesScene, "Particles");
