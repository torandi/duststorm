#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "particle_system.hpp"
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
		, skybox("skydark")
		, fire(2000,  TextureArray::from_filename("fire1.png", "fire2.png", "fire3.png", nullptr))
		, smoke(2000, TextureArray::from_filename("fog.png", nullptr), 20) {

		camera.set_position(glm::vec3(-5.f, 0.f, 10.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));

		lights.ambient_intensity() = glm::vec3(0.01f);
		lights.num_lights() = 2;
		lights.lights[0].set_position(glm::vec3(2.f, 0.2f, 1.f));
		lights.lights[0].intensity = glm::vec3(0.9f, 0.3f, 0.0f);
		lights.lights[0].type = Light::POINT_LIGHT;
		lights.lights[0].constant_attenuation = 0.0f;
		lights.lights[0].linear_attenuation = 0.1f;
		lights.lights[0].quadratic_attenuation = 0.8f;

		lights.lights[1].set_position(glm::vec3(-2.f, 1.f, 1.f));
		lights.lights[1].intensity = glm::vec3(0.4f, 0.8f, 0.8f);
		lights.lights[1].type = Light::POINT_LIGHT;

		//Fire configuration:
		fire.config.spawn_position = glm::vec4(0.f, 0.f, 0.f, 1.f);
		fire.config.spawn_area = glm::vec4(0.4f, 0.f, 0.4f, 0.f);
		fire.config.spawn_direction = glm::vec4(0, 1.f, 0.f, 0.f);
		fire.config.direction_var = glm::vec4(0.1f, 0.f, 0.1f, 0.f);
		fire.config.avg_spawn_speed= 0.001f;
		fire.config.spawn_speed_var = 0.0005f;
		fire.config.avg_ttl = 10.f;
		fire.config.ttl_var = 2.5f;
		fire.config.avg_spawn_delay = 0.f;
		fire.config.spawn_delay_var = 0.f;
		fire.config.avg_scale = 0.4f;
		fire.config.scale_var = 0.05f;
		fire.config.avg_scale_change = 0.2f;
		fire.config.scale_change_var = 0.05f;
		fire.config.avg_rotation_speed = 0.1f;
		fire.config.rotation_speed_var = 0.05f;
		fire.config.birth_color = glm::vec4(1.0, 0.8, 0.0, 1.0);
		fire.config.death_color = glm::vec4(0.8 ,0.2, 0.1, 0.f);
		fire.config.motion_rand = glm::vec4(0.0, 0.f, 0.0, 0);
		fire.update_config();

		fire.set_position(glm::vec3(2.f, -0.5f, 1.f));
		smoke.set_position(glm::vec3(2.f, -0.5f, 1.f));

		smoke.config.spawn_position = glm::vec4(0.f, 0.f, 0.f, 1.f);
		smoke.config.spawn_area = glm::vec4(0.4f, 0.f, 0.4f, 0.f);
		smoke.config.spawn_direction = glm::vec4(0, 1.f, 0.f, 0.f);
		smoke.config.direction_var = glm::vec4(0.1f, 0.f, 0.1f, 0.f);
		smoke.config.avg_spawn_speed= 0.001f;
		smoke.config.spawn_speed_var = 0.0005f;
		smoke.config.avg_ttl = 18.f;
		smoke.config.ttl_var = 5.f;
		smoke.config.avg_spawn_delay = 0.f;
		smoke.config.spawn_delay_var = 0.f;
		smoke.config.avg_scale = 0.8f;
		smoke.config.scale_var = 0.05f;
		smoke.config.avg_scale_change = 3.f;
		smoke.config.scale_change_var = 0.5f;
		smoke.config.avg_rotation_speed = 0.02f;
		smoke.config.rotation_speed_var = 0.005f;
		smoke.config.birth_color = glm::vec4(0.2, 0.2, 0.2, 0.3);
		smoke.config.death_color = glm::vec4(0.8 ,0.8, 0.8, 0.f);
		smoke.config.motion_rand = glm::vec4(0.001f, 0.f, 0.001f, 0);
		smoke.update_config();
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
			tv_test.render();
			tv_room.render();
		}
		shaders[SHADER_PARTICLES]->bind();
		{
			smoke.render();
			fire.render();
		}
		Shader::unbind();
	}

	virtual void update(float t, float dt){
		camera.set_position(v.at(t));
		fire.update(dt);
		smoke.update(dt);
	}

private:
	RenderObject tv_test;
	RenderObject tv_room;
	Shader::lights_data_t light_data;
	Light* light[1];
	Camera camera;
	PointTable v;
	Skybox skybox;
	ParticleSystem fire, smoke;
};

template <>
SceneFactory::Metadata* SceneTraits<TVScene>::metadata(){
	SceneFactory::Metadata* _ = new SceneFactory::Metadata;
	SceneFactory::Metadata& m = *_;
	m["Camera 1"] = "path:tv_cam1.txt";
	m["TV model"] = "model:tv.obj";
	m["Room model"] = "model:tv_room.obj";
	m["Light[0]"] = "light:tv_light0.txt";
	m["Float"] = "float:test1";
	m["Int"] = "int:test2";
	m["Vec3"] = "vec3:test3";
	return _;
}

REGISTER_SCENE_TYPE(TVScene, "TV", "tv.meta");
