#ifndef ENGINE_H
#define ENGINE_H

namespace Engine {

	/**
	 * Enable and disable settings.
	 */
	void setup_opengl();

	/**
	 * Load all shaders.
	 */
	void load_shaders();

	/**
	 * Register all scene types.
	 */
	void autoload_scenes();
};

#endif /* ENGINE_H */
