#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "shader.hpp"
#include "timetable.hpp"
#include "skybox.hpp"

class TVScene: public Scene {
public:
	TVScene(const glm::ivec2& size)
		: Scene(size)
		, tv_test("tv.obj", false)
		, tv_room("tv_room.obj", false)
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.0f)
		, v("scene/tv_cam1.txt")
		, skybox("skydark") {

		camera.set_position(glm::vec3(0.f, 0.f, -1.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));

		lights.ambient_intensity() = glm::vec3(0.01f);
		lights.num_lights() = 2;
		lights.lights[0]->set_position(glm::vec3(2.f, 1.f, 1.f));
		lights.lights[0]->intensity = glm::vec3(0.8f);
		lights.lights[0]->type = Light::POINT_LIGHT;
		lights.lights[0]->linear_attenuation = 0.2f;

		lights.lights[1]->set_position(glm::vec3(-2.f, 1.f, 1.f));
		lights.lights[1]->intensity = glm::vec3(0.4f, 0.8f, 0.8f);
		lights.lights[1]->type = Light::POINT_LIGHT;
	}

	virtual void render(){
		clear(Color::green);
	
		Shader::upload_lights(lights);
		shaders[SHADER_SKYBOX]->bind();
		{
			skybox.render(camera);
		}
		Shader::upload_camera(camera);
		shaders[SHADER_NORMAL]->bind();
		{
			tv_test.render(shaders[SHADER_NORMAL]);
			tv_room.render(shaders[SHADER_NORMAL]);
		}
		Shader::unbind();
	}

	virtual void update(float t, float dt){
		camera.set_position(v.at(t));
	}

private:
	RenderObject tv_test;
	RenderObject tv_room;
	Camera camera;
	PointTable v;
	Skybox skybox;
};

template <>
SceneFactory::Metadata* SceneTraits<TVScene>::metadata(){
	SceneFactory::Metadata* _ = new SceneFactory::Metadata;
	SceneFactory::Metadata& m = *_;
	m["Camera 1"] = "camera:tv_cam1.txt";
	m["TV model"] = "model:tv.obj";
	return _;
}

REGISTER_SCENE_TYPE(TVScene, "TV");
