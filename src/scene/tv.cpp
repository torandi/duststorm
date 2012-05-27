#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "shader.hpp"

class TVScene: public Scene {
public:
	TVScene(const glm::ivec2& size)
		: Scene(size)
		, tv_test("tv.obj"){
	}

	static Scene* factory(const glm::ivec2& size){
		return new TVScene(size);
	}

	virtual void render(){
		clear(Color::green);
		shaders[SHADER_NORMAL]->bind();
		{
			tv_test.render(shaders[SHADER_NORMAL]);
		}
		shaders[SHADER_NORMAL]->unbind();
	}

	virtual void update(float t, float dt){
		tv_test.yaw(M_PI_4 * dt);
	}

private:
	RenderObject tv_test;
};

REGISTER_SCENE_TYPE(TVScene, "TV");
