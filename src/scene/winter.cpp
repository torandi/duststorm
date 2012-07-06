#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shader.hpp"
#include "scene.hpp"
#include "globals.hpp"
#include "particle_system.hpp"
#include "quad.hpp"
#include "texture.hpp"
#include "terrain.hpp"

class WinterScene : public Scene {
public:
	WinterScene (const glm::ivec2 &size)
		: Scene(size)
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.f) {
			camera.set_position(glm::vec3(0.f, 0.f, -1));
			camera.look_at(glm::vec3(0.f, 0.f, 0.f));
			terrain_shader = Shader::create_shader("terrain");
			TextureArray * color = TextureArray::from_filename("dirt.png","grass.png", nullptr);
			//TextureArray * normal = TextureArray::from_filename("dirt_normal.png","grass_normal.png");
			TextureArray * normal = TextureArray::from_filename("default_normalmap.jpg","default_normalmap.jpg", nullptr);
			terrain = new Terrain("park", 1.f, 20.f, color, normal);
			terrain->absolute_move(glm::vec3(0.f, -10.f, 0.f));

			lights.ambient_intensity() = glm::vec3(0.05f);
			lights.num_lights() = 1;
			lights.lights[0].set_position(glm::vec3(5, 0.8f, 6.f));
			lights.lights[0].intensity = glm::vec3(0.8f);
			lights.lights[0].type = Light::DIRECTIONAL_LIGHT;
	}

	virtual void render_geometry(const Camera& cam){
		terrain_shader->bind();
		Shader::upload_lights(lights);
		Shader::upload_camera(camera);
		terrain->render();
	}

	virtual void render(){
		clear(Color::magenta);
		glDisable(GL_CULL_FACE);
		render_geometry(camera);
		Shader::unbind();
	}

	virtual const Camera& get_current_camera(){
		return camera;
	}

	virtual void update(float t, float dt){
		#ifdef ENABLE_INPUT
			Input::movement_speed = 5.f;
			input.update_object(camera, dt);
			if(input.current_value(Input::ACTION_1) > 0.5f) {
				printf("Current position: (%f, %f, %f)\n", camera.position().x, camera.position().y, camera.position().z);
			}
		#endif
	}

private:
	Camera camera;
	Shader * terrain_shader;
	Terrain * terrain;
};

REGISTER_SCENE_TYPE(WinterScene, "Winter", "winter.meta");
