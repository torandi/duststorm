#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "shader.hpp"

class TestScene: public Scene {
public:
	TestScene(const glm::ivec2& size):
		Scene(size){

	}

	virtual void render(){
		clear(Color::blue);
	}
};

REGISTER_SCENE_TYPE(TestScene, "Test", "test.meta");
