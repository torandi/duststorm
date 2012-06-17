#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glm/gtc/type_ptr.hpp>

#include "quad.hpp"
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
		, cam_pos1("scene/nox_cam1.txt")
		, cam_pos2("scene/nox_cam2.txt")
		, skybox("skydark") 
		, water_quad(10.f, true, true)
		, water_texture(Texture2D::from_filename("water.png")) 
		, fog(100000, TextureArray::from_filename("fog.png", nullptr))
	{

		logo.set_scale(0.1f);
		logo.set_rotation(glm::vec3(0,1,0), 90.0f);
		logo.set_position(glm::vec3(-30,1,0));

		camera.set_position(glm::vec3(13.f, 11.f, 0.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));

		water_quad.set_position(glm::vec3(-100.f, -0.6f, -50.f));
		water_quad.set_rotation(glm::vec3(1.f, 0, 0), 90.f);
		water_quad.set_scale(100.f);

		water_texture->texture_bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
		water_texture->texture_unbind();

		u_wave1 = shaders[SHADER_WATER]->uniform_location("wave1");
		u_wave2 = shaders[SHADER_WATER]->uniform_location("wave2");

		wave1 = glm::vec2(0.01, 0);
		wave2 = glm::vec2(0.005, 0.03);

		lights.ambient_intensity() = glm::vec3(0.01f);
		lights.num_lights() = 2;

		lights.lights[0].set_position(glm::vec3(-5.5f, 0.4f, 0.0f));
		lights.lights[0].intensity = glm::vec3(0.0f, 0.6f, 0.4f);
		lights.lights[0].type = Light::POINT_LIGHT;
		lights.lights[0].constant_attenuation = 0.0f;
		lights.lights[0].linear_attenuation = 0.1f;
		lights.lights[0].quadratic_attenuation = 0.4f;

		lights.lights[1].set_position(glm::vec3(-2.0f, 1.0f, 0.0f));
		lights.lights[1].intensity = glm::vec3(0.3f, 0.6f, 0.8f);
		lights.lights[1].type = Light::POINT_LIGHT;

		fog.avg_spawn_rate = 50000.f;
		fog.spawn_rate_var = 0.f;

		fog.config.spawn_position = glm::vec4(-50.f, -1.f, -30.f, 1.f);
	//	fog.config.spawn_position = glm::vec4(0.f, 0.f, 0.f, 1.f);
		fog.config.spawn_area = glm::vec4(50.0f, 0.f, 60.0f, 0.0f);
		fog.config.spawn_direction = glm::vec4(0, 0.f, 0.f, 1.f);
		fog.config.direction_var = glm::vec4(0.3f, 0.3f, 0.3f, 0.f);
		fog.config.avg_spawn_speed= 0.0001f;
		fog.config.spawn_speed_var = 0.0003f;
		fog.config.avg_ttl = 20.f;
		fog.config.ttl_var = 10.f;
		fog.config.avg_scale = 15.f;
		fog.config.scale_var = 5.f;
		fog.config.avg_scale_change = 0.5f;
		fog.config.scale_change_var = 0.1f;
		fog.config.avg_rotation_speed = 0.02f;
		fog.config.rotation_speed_var = 0.005f;
		fog.config.birth_color = glm::vec4(0.3f, 0.3f, 0.3f, 0.05);
		fog.config.death_color = glm::vec4(0.3f ,0.3f, 0.3f, 0.0f);
		fog.config.motion_rand = glm::vec4(0.001f, 0.f, 0.001f, 0);
		fog.update_config();
	}

	virtual void render_geometry(const Camera& cam){
		clear(Color::black);
		Shader::upload_lights(lights);

		/*shaders[SHADER_SKYBOX]->bind();
		skybox.render(cam);*/

		Shader::upload_camera(cam);
		shaders[SHADER_NORMAL]->bind();

		tunnel.render();
		logo.render();

		glActiveTexture(GL_TEXTURE0);
		water_texture->texture_bind();
		glActiveTexture(GL_TEXTURE2);
		skybox.texture->texture_bind();

		shaders[SHADER_WATER]->bind();
		{
			glUniform2fv(u_wave1, 1, glm::value_ptr(wave1));
			glUniform2fv(u_wave2, 1, glm::value_ptr(wave2));
			water_quad.render();
		}

		skybox.texture->texture_unbind();
		glActiveTexture(GL_TEXTURE0);
		water_texture->texture_unbind();

		shaders[SHADER_PARTICLES]->bind();
		fog.render();

	}

	virtual const Camera& get_current_camera(){
		return camera;
	}

	virtual void update(float t, float dt){
		camera.set_position(cam_pos1.at(t));
		camera.look_at(cam_pos2.at(t));

		fog.update(dt);
	}

	RenderObject tunnel;
	RenderObject logo;
	Camera camera;
	PointTable cam_pos1;
	PointTable cam_pos2;
	Skybox skybox;

	Quad water_quad;
	Texture2D* water_texture;
	glm::vec2 wave1, wave2;
	GLint u_wave1, u_wave2;
	ParticleSystem fog;
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
	//m["fog"]				= new MetaParticles<NOX>(&NOX::fog
	return _;
}

REGISTER_SCENE_TYPE(NOX, "NördtroXy II", "nox.meta");
