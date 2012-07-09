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
			/*terrain_shader = Shader::create_shader("terrain");
			TextureArray * color = TextureArray::from_filename("dirt.png","grass.png");
			TextureArray * normal = TextureArray::from_filename("dirt_normal.png","grass_normal.png");
			terrain = new Terrain("park", 10.f, 10.f, color, normal);*/
	}

	virtual void render_geometry(const Camera& cam){
		terrain_shader->bind();
		Shader::upload_camera(camera);
		//terrain->render();
	}

	virtual void render(){
		clear(Color::red);
		render_geometry(camera);
		Shader::unbind();
	}

	virtual const Camera& get_current_camera(){
		return camera;
	}

	virtual void update(float t, float dt){
		#ifdef ENABLE_INPUT
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
