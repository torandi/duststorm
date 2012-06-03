#ifndef ENGINE_H
#define ENGINE_H

#include <string>

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

	/**
	 * Get a rendertarget by name.
	 * @return nullptr if not found.
	 */
	RenderTarget* rendertarget_by_name(const std::string& name);
};

#endif /* ENGINE_H */
