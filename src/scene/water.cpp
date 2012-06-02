#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "timetable.hpp"
#include "quad.hpp"
#include "texture.hpp"

class WaterScene: public Scene {
public:
	WaterScene(const glm::ivec2& size)
		: Scene(size)
		, quad()
		, water(Texture2D::from_filename("hest.jpg"))
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.0f) {

		camera.set_position(glm::vec3(0.f, 0.f, -1.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));
		quad.set_position(glm::vec3(0, 0, 0.f));
	}

	virtual void render(){
		clear(Color::green);

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
	Camera camera;
};

REGISTER_SCENE_TYPE(WaterScene, "Water");
