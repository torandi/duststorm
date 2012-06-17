#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "light.hpp"
#include "render_object.hpp"
#include "particle_system.hpp"
#include "shader.hpp"
#include "timetable.hpp"
#include "skybox.hpp"

class NOX: public Scene {
public:
	NOX(const glm::ivec2& size)
		: Scene(size)
		, tunnel("nox2/tunnel.obj", false)
		, logo("nox.obj", false)
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.0f)
		, v("scene/tv_cam1.txt")
		, skybox("skydark") {

		logo.set_scale(0.1f);
		logo.set_rotation(glm::vec3(0,1,0), 90.0f);
		logo.set_position(glm::vec3(-30,1,0));

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
	}

	virtual bool meta_set(const std::string& key, const std::string& value){
		return false;
	}

	virtual void render(){
		clear(Color::white);

		Shader::upload_lights(lights);
		shaders[SHADER_SKYBOX]->bind();
		skybox.render(camera);

		Shader::upload_camera(camera);
		shaders[SHADER_NORMAL]->bind();
		tunnel.render();
		logo.render();

		Shader::unbind();
	}

	virtual void update(float t, float dt){
		camera.set_position(v.at(t));
	}

	RenderObject tunnel;
	RenderObject logo;
	Camera camera;
	PointTable v;
	Skybox skybox;
};

template <>
SceneFactory::Metadata* SceneTraits<NOX>::metadata(){
	SceneFactory::Metadata* _ = new SceneFactory::Metadata;
	SceneFactory::Metadata& m = *_;
	m["Camera 1"]   = new MetaPath("nox_cam1.txt");
	m["Tunnel"]     = new MetaModel("nox2/tunnel.obj");
	m["Logo"]       = new MetaModel("nox.obj");
	m["Light[0]"]   = new MetaLight<NOX>(&NOX::lights, 0, "tv_light0.txt");
	m["Light[1]"]   = new MetaLight<NOX>(&NOX::lights, 1, "tv_light1.txt");
	return _;
}

REGISTER_SCENE_TYPE(NOX, "NördtroXy II", "nox.meta");
