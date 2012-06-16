#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include <cstring>
#include <sstream>

static SceneFactory::SceneMap map;

namespace SceneFactory {

	void register_factory(const std::string& name, factory_callback func, Metadata* meta, const std::string& filename){
		map[name] = {meta, func, name, filename};
	}

	Scene* create(const std::string& name, const glm::ivec2& size){
		auto it = map.find(name);
		if ( it == map.end() ){
			return nullptr;
		}

		Scene* scene = it->second.func(size);
		scene->meta_load(&it->second);
		return scene;
	}

	SceneMap::const_iterator begin(){
		return map.begin();
	}

	SceneMap::const_iterator end(){
		return map.end();
	}

	SceneMap::const_iterator find(const std::string& name){
		return map.find(name);
	}
}
