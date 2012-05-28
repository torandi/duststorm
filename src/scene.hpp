#ifndef SCENE_H
#define SCENE_H

#include "rendertarget.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

/**
 * Scene definition.
 */
class Scene: public RenderTarget {
public:
	Scene(const glm::ivec2& size);
	Scene(size_t width, size_t height);
	virtual ~Scene();

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

namespace SceneFactory {
	typedef std::map<std::string, std::string> Metadata;
	typedef Scene*(*factory_callback)(const glm::ivec2& size);
}

/**
 * Static information about a scene-type.
 */
struct SceneInfo {
	/**
	 * Metadata is a map of resources the scene depends on.
	 * Key is the name of the resource.
	 * Value is a TYPE:DATA pair where TYPE defines what kind of resource it is.
	 * E.g. "Camera 1" -> "camera:points.txt"
	 */
	SceneFactory::Metadata* meta;

	/**
	 * Function used to allocate a new instance.
	 */
	SceneFactory::factory_callback func;
};

namespace SceneFactory {
	typedef std::map<std::string, SceneInfo> SceneMap;

	/**
	 * Allocate a new scene by typename.
	 * Name must match a previously registered type.
	 * @see REGISTER_SCENE_TYPE
	 * @return nullptr if no matching scene could be found.
	 */
	Scene* create(const std::string& name, const glm::ivec2& size);

	/**
	 * Scene-type iterator.
	 */
	SceneMap::const_iterator begin();
	SceneMap::const_iterator end();

	/**
	 * Register a new scene class. Do not call directly, use REGISTER_SCENE_TYPE.
	 */
	void register_factory(const std::string& name, factory_callback func, Metadata* meta);
};

/**
 * Glue between REGISTER_SCENE_TYPE and SceneFactory.
 * Specialize this class if you need to implement a custom function.
 */
template <class T>
class SceneTraits {
public:

	/**
	 * Create a new instance.
	 */
	static Scene* factory(const glm::ivec2& size){
		return new T(size);
	}

	/**
	 * Allocate metadata. Default is an empty map.
	 */
	static SceneFactory::Metadata* metadata(){
		return new SceneFactory::Metadata;
	}
};

/**
 * Register a new scene-type which can be allocated using name.
 */
#define REGISTER_SCENE_TYPE(cls, name) \
	void _register_##cls () { \
		SceneFactory::register_factory(name, SceneTraits<cls>::factory, SceneTraits<cls>::metadata()); \
	}

#endif /* SCENE_H */
