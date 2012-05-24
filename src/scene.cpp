#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"

Scene::Scene(const glm::ivec2& size)
	: RenderTarget(size, false)
	, current(timetable.end())
	, match(false) {
}

Scene::Scene(size_t width, size_t height)
	: RenderTarget(glm::ivec2(width, height), false)
	, current(timetable.end())
	, match(false) {

}

Scene::~Scene(){

}

void Scene::add_time(float begin, float end){
	struct time tmp = {begin, end};
	timetable.push_back(tmp);
	current = timetable.begin();
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
