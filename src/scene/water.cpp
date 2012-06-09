#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
		, quad(10.f)
		, water(Texture2D::from_filename("water.png"))
		, skybox("night")
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.0f) {

		camera.set_position(glm::vec3(0.f, 0.f, -1.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));
		quad.set_position(glm::vec3(-0.5f, -0.5f, 0.f));
		quad.set_rotation(glm::vec3(1.f, 0, 0), 90.f);
		quad.set_scale(10.f);

		lights.ambient_intensity() = glm::vec3(0.01f);
		lights.num_lights() = 1;
		lights.lights[0]->set_position(glm::vec3(0, 0.5f, 0.f));
		lights.lights[0]->intensity = glm::vec3(0.8f);
		lights.lights[0]->type = Light::POINT_LIGHT;

		water->texture_bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		water->texture_unbind();
	}

	virtual void render(){
		clear(Color::green);

		Shader::upload_lights(lights);

		shaders[SHADER_SKYBOX]->bind();
		skybox.render(camera);

		Shader::upload_camera(camera);

		Shader::upload_blank_material();
		glActiveTexture(GL_TEXTURE0);
		water->texture_bind();
		shaders[SHADER_NORMAL]->bind();
		{
			quad.render();
		}
		Shader::unbind();
		water->texture_unbind();
	}

	virtual void update(float t, float dt){
	}

private:
	Quad quad;
	Texture2D* water;
	Skybox skybox;
	Camera camera;
};

REGISTER_SCENE_TYPE(WaterScene, "Water");
