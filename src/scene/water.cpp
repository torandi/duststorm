#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "timetable.hpp"
#include "quad.hpp"
#include "texture.hpp"
#include "skybox.hpp"

class WaterScene: public Scene {
public:
	WaterScene(const glm::ivec2& size)
		: Scene(size)
		, quad(5.f, true, true)
		, water(Texture2D::from_filename("water.png"))
		, skybox("skydark")
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.0f)
		, time(0.f) {

		camera.set_position(glm::vec3(0.f, 0.5f, 0.f));
		camera.look_at(glm::vec3(0.f, 0.f, 4.f));
		quad.set_position(glm::vec3(-25.f, 0.0f, 0.f));
		quad.set_rotation(glm::vec3(1.f, 0, 0), 90.f);
		quad.set_scale(50.f);

		lights.ambient_intensity() = glm::vec3(0.05f);
		lights.num_lights() = 1;
		lights.lights[0]->set_position(glm::vec3(0, 0.5f, 0.f));
		lights.lights[0]->intensity = glm::vec3(0.8f);
		lights.lights[0]->type = Light::POINT_LIGHT;

		water->texture_bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
		water->texture_unbind();

		u_wave1 = shaders[SHADER_WATER]->uniform_location("wave1");
		u_wave2 = shaders[SHADER_WATER]->uniform_location("wave2");

		wave1 = glm::vec2(0.01, 0);
		wave2 = glm::vec2(0.005, 0.03);
	}

	virtual void render(){
		clear(Color::black);

		Shader::upload_lights(lights);

		//shaders[SHADER_SKYBOX]->bind();
		//skybox.render(camera);

		Shader::upload_camera(camera);

		//Shader::upload_blank_material();
		glActiveTexture(GL_TEXTURE0);
		water->texture_bind();
		glActiveTexture(GL_TEXTURE2);
		skybox.texture->texture_bind();

		shaders[SHADER_WATER]->bind();
		{
			glUniform2fv(u_wave1, 1, glm::value_ptr(wave1));
			glUniform2fv(u_wave2, 1, glm::value_ptr(wave2));
			quad.render();
		}
		Shader::unbind();

		skybox.texture->texture_bind();
		glActiveTexture(GL_TEXTURE0);
		water->texture_unbind();
	}

	virtual void update(float t, float dt){
		time+=dt;
	}

private:
	Quad quad;
	Texture2D* water;
	Skybox skybox;
	Camera camera;
	float time;
	glm::vec2 wave1, wave2;
	GLint u_time, u_wave1, u_wave2;
};

REGISTER_SCENE_TYPE(WaterScene, "Water");
