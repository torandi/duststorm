#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include <cstdio>

Scene::Scene(const glm::ivec2& size)
	: target(nullptr)
	, current(timetable.end())
	, match(false) {

	target = new RenderTarget(size, false);
	if ( !target ){
		fprintf(stderr, "Failed to create Scene::RenderTarget\n");
		abort();
	}
}

Scene::~Scene(){
	delete target;
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
}

GLuint Scene::texture() const {
	return target->texture();
}

const RenderTarget* Scene::rendertarget() const {
	return target;
}
