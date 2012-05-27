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
		, particles(100000)
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.f) {
			camera.set_position(glm::vec3(0.f, 0.f, -1));
			camera.look_at(glm::vec3(0.f, 0.f, 0.f));
			particles.config.spawn_position = glm::vec4(-1.f, -0.5f, 0.f, 1.f);
			particles.update_config();
	}

	static Scene* factory(const glm::ivec2& size) {
		return new ParticlesScene(size);
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
