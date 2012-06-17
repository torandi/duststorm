#ifndef ENGINE_H
#define ENGINE_H

#include <string>

namespace Engine {

	void init();
	void cleanup();
	void update(float t, float dt);
	void render();

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

	/**
	 * Get a rendertarget by name.
	 * @return nullptr if not found.
	 */
	RenderTarget* rendertarget_by_name(const std::string& name);

	/**
	 * Load timetable from file. Uses rendertaget_by_name to locate scenes.
	 */
	void load_timetable(const std::string& filename);
};

#endif /* ENGINE_H */
