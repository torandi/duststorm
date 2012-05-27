#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "shader.hpp"
#include "timetable.hpp"

class TVScene: public Scene {
public:
	TVScene(const glm::ivec2& size)
		: Scene(size)
		, tv_test("tv.obj")
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.0f)
		, v("scene/tv_cam1.txt") {

		camera.set_position(glm::vec3(0.f, 0.f, -1.f));
		camera.look_at(glm::vec3(0.f, 0.f, 0.f));
	}

	static Scene* factory(const glm::ivec2& size){
		return new TVScene(size);
	}

	static Metadata* metadata(){
		Metadata* _ = new Metadata;
		Metadata& m = *_;
		m["Camera 1"] = "camera:tv_cam1.txt";
		m["TV model"] = "model:tv.obj";
		return _;
	}

	virtual void render(){
		clear(Color::green);

		Shader::upload_camera(camera);
		shaders[SHADER_NORMAL]->bind();
		{
			tv_test.render(shaders[SHADER_NORMAL]);
		}
		Shader::unbind();
	}

	virtual void update(float t, float dt){
		//tv_test.set_rotation(glm::vec3(0.0f,1.0f,0.0f), M_PI_4 * t);
		camera.set_position(v.at(t));
	}

private:
	RenderObject tv_test;
	Camera camera;
	PointTable v;
};

REGISTER_SCENE_TYPE(TVScene, "TV");
