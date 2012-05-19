#ifndef SCENE_H
#define SCENE_H

#include "rendertarget.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Scene {
public:
	Scene(const glm::ivec2& size);
	virtual ~Scene();

	/**
	 * Setup timetable for the scene.
	 */
	void add_time(float begin, float end);

	/**
	 * Update scene state.
	 * @param t Absolute time.
	 * @param dt Delta since last update.
	 */
	virtual void update(float t, float dt);

	/**
	 * Called by render_scene to do drawing calls.
	 */
	virtual void render();

	/**
	 * Update scene if active.
	 * Do not override.
	 */
	void update_scene(float t, float dt);

	/**
	 * Render scene onto target.
	 * Do not override.
	 */
	void render_scene();

	/**
	 * Get scene texture (from render target).
	 */
	GLuint texture() const;

	const RenderTarget* rendertarget() const;

private:
	struct time {
		float begin;
		float end;
	};

	std::vector<time> timetable;
	std::vector<time>::iterator current;
	RenderTarget* target;
	bool match;
};

#endif /* SCENE_H */
