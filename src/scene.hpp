#ifndef SCENE_H
#define SCENE_H

#include "rendertarget.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

class Scene: public RenderTarget {
public:
	Scene(const glm::ivec2& size);
	Scene(size_t width, size_t height);
	virtual ~Scene();

	/**
	 * Allocate a new scene by typename.
	 * Name must match a previously registered type.
	 * @see REGISTER_SCENE_TYPE
	 * @return nullptr if no matching scene could be found.
	 */
	static Scene* create(const std::string& name, const glm::ivec2& size);

	/**
	 * Setup timetable for the scene.
	 * @return this to allow chaining.
	 */
	Scene* add_time(float begin, float end);

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

	bool is_active() const;

	typedef Scene*(*factory_callback)(const glm::ivec2& size);
	static void register_factory(const std::string& name, factory_callback func);
	static std::map<std::string, factory_callback>::const_iterator factory_begin();
	static std::map<std::string, factory_callback>::const_iterator factory_end();

protected:
	float stage(float t) const;

private:
	struct time {
		float begin;
		float end;
	};

	std::vector<time> timetable;
	std::vector<time>::iterator current;
	bool match;
};

/**
 * Register a new scene-type which can be allocated using name.
 */
#define REGISTER_SCENE_TYPE(cls, name) void _register_##cls () { Scene::register_factory(name, cls::factory); }

#endif /* SCENE_H */
