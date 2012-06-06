#ifndef SCENE_H
#define SCENE_H

#include "rendertarget.hpp"
#include "lights_data.hpp"
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
	Scene(const glm::ivec2& size, GLenum format = GL_RGB8);
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

	/**
	 * Load scene metadata.
	 * Called automatically by SceneFactory::create.
	 */
	void meta_load(struct SceneInfo* info);

	/**
	 * Update metadata.
	 * @return true if successful.
	 */
	virtual bool meta_set(const std::string& key, const std::string& value);

	virtual std::string meta_get(const std::string& key) const;

	void meta_persist();

	LightsData lights;

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

class Meta {
public:
	enum Type {
		TYPE_MODEL,
		TYPE_PATH,
		TYPE_LIGHT,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_VEC2,
		TYPE_VEC3,
		TYPE_VEC4,
		TYPE_STRING,
		TYPE_COLOR,
	};

	Meta(Type type): type(type){}
	const Type type;

	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset) = 0;
	virtual std::string get_string(Scene* instance, unsigned int offset) const = 0;
};

class MetaFile: public Meta {
protected:
	MetaFile(const std::string& filename, Type type): Meta(type){}

public:
	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset){ return str; };
	virtual std::string get_string(Scene* instance, unsigned int offset) const { return ""; }
};

class MetaModel: public MetaFile {
public:
	MetaModel(const std::string& filename): MetaFile(filename, TYPE_MODEL){}
};

class MetaPath: public MetaFile {
public:
	MetaPath(const std::string& filename): MetaFile(filename, TYPE_PATH){}
};

class MetaLightBase: public MetaFile {
public:
	MetaLightBase(const std::string& filename)
		: MetaFile(filename, TYPE_LIGHT){
	}

	virtual MovableLight& get(Scene* instance) const = 0;
	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset);
	virtual std::string get_string(Scene* instance, unsigned int offset) const;
};

template <class cls>
class MetaLight: public MetaLightBase {
public:
	MetaLight(LightsData cls::*light, unsigned int index, const std::string& filename)
		: MetaLightBase(filename)
		, light(light)
		, index(index) {
	}

	virtual MovableLight& get(Scene* instance) const { return (dynamic_cast<cls*>(instance)->*light).lights[index]; }

private:
	LightsData cls::*light;
	unsigned int index;
};

template <class T>
class MetaVariableBase: public Meta {
public:
	MetaVariableBase();
	virtual void set(Scene* instance, const T& value) = 0;
	virtual T get(Scene* instance) const = 0;

	virtual std::string set_string(Scene* instance, const std::string& str, unsigned int offset);
	virtual std::string get_string(Scene* instance, unsigned int offset) const;
};

template <class T, class cls>
class MetaVariable: public MetaVariableBase<T> {
public:
	MetaVariable(T cls::*ptr): ptr(ptr){}
	virtual void set(Scene* instance, const T& value){ dynamic_cast<cls*>(instance)->*ptr = value; }
	virtual T get(Scene* instance) const {
		const cls* tmp = dynamic_cast<cls*>(instance);
		return tmp->*ptr;
	}
private:
	T cls::*ptr;
};

namespace SceneFactory {
	typedef std::map<std::string, Meta*> Metadata;
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

	/**
	 * Name of scene.
	 */
	std::string name;

	/**
	 * Filename holding metadata for the scene.
	 */
	std::string filename;
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
	SceneMap::const_iterator find(const std::string& name);

	/**
	 * Register a new scene class. Do not call directly, use REGISTER_SCENE_TYPE.
	 */
	void register_factory(const std::string& name, factory_callback func, Metadata* meta, const std::string& filename);
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
#define REGISTER_SCENE_TYPE(cls, name, filename) \
	void _register_##cls () { \
		SceneFactory::register_factory(name, SceneTraits<cls>::factory, SceneTraits<cls>::metadata(), filename); \
	}

#endif /* SCENE_H */
