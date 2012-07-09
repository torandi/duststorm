#ifndef ENGINE_H
#define ENGINE_H

#include <string>

namespace Engine {

	void init();
	void cleanup();
	void update(float dt);
	void render();

	/**
	 * Enable and disable settings.
	 */
	void setup_opengl();

	void load_shaders();
};

#endif /* ENGINE_H */
