#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "shader.hpp"

class TestScene: public Scene {
public:
	TestScene(const glm::ivec2& size)
		: Scene(size)
		, camera(60, 1, 0.1, 100) {

	}

	virtual void render_geometry(const Camera& cam){

	}

	virtual void render(){
		clear(Color::blue);
	}

	virtual const Camera& get_current_camera(){
		return camera;
	}

	Camera camera;
};

REGISTER_SCENE_TYPE(TestScene, "Test", "test.meta");
