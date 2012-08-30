#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <SDL/SDL.h>

namespace Engine {

	void init();
	void cleanup();
	void update(float dt);
	void render();

	void terminate(); //Implemented in main.cpp

	/**
	 * Enable and disable settings.
	 */
	void setup_opengl();

	void load_shaders();
	void input(const SDL_Event &event);
};

#endif /* ENGINE_H */
