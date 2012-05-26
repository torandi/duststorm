#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include <map>

static std::map<std::string, Scene::factory_callback> factory_map;

Scene::Scene(const glm::ivec2& size)
	: RenderTarget(size, false, true)
	, current(timetable.end())
	, match(false) {
}

Scene::Scene(size_t width, size_t height)
	: RenderTarget(glm::ivec2(width, height), false, true)
	, current(timetable.end())
	, match(false) {

}

Scene::~Scene(){

}

Scene* Scene::add_time(float begin, float end){
	struct time tmp = {begin, end};
	timetable.push_back(tmp);
	current = timetable.begin();
	return this;
}

void Scene::update(float t, float dt){

}

void Scene::render(){

}

void Scene::update_scene(float t, float dt){
	/* find current timetable entry */
	struct time c;

	/* sanity check (no entries) */
	if ( current == timetable.end() ){
		return;
	}

	do {
		c = *current;
		if ( dt > 0.0f && t > c.end && current+1 != timetable.end() ){
			/* current time is ahead of current entry, move to the next */
			++current;
			continue;
		} else if ( dt < 0.0f && t < c.begin && current != timetable.begin()){
			/* current time is before current entry, move back (time flows backwards) */
			--current;
			continue;
		}
		break;
	} while (true);

	/* test if it matches timetable */
	match = !(t < c.begin || t > c.end);
	if ( !match ) return;

	/* actually update scene */
	update(t, dt);
}

void Scene::render_scene(){
	if ( !match ) return;
	with(std::bind(&Scene::render, this));
}

float Scene::stage(float t) const {
	struct time c = *current;
	const float a = t - c.begin;
	const float b = c.end - c.begin;
	return a / b;
}

bool Scene::is_active() const {
	return match;
}

void Scene::register_factory(const std::string& name, factory_callback func){
	factory_map[name] = func;
}

Scene* Scene::create(const std::string& name, const glm::ivec2& size){
	auto it = factory_map.find(name);
	if ( it == factory_map.end() ){
		return nullptr;
	}
	return it->second(size);
}
