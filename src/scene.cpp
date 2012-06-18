#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "scene.hpp"
#include <cstring>

Scene::Scene(const glm::ivec2& size, GLenum format)
	: RenderTarget(size, format, true)
	, match(false)
	, current(timetable.end())
	, meta_definition(nullptr) {
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
	glClear(GL_DEPTH_BUFFER_BIT);
	render_geometry(get_current_camera());
	Shader::unbind();
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

void Scene::meta_load(struct SceneInfo* info){
	const std::string filename = PATH_SRC "scene/" + info->filename;
	FILE* fp = fopen(filename.c_str(), "r");
	if ( !fp ){
		fprintf(stderr, "Failed to read metadata for scene `%s' from `%s': %s\n", info->name.c_str(), filename.c_str(), strerror(errno));
		return;
	}

	char empty[] = "";
	char* buffer = nullptr;
	size_t len = 0;
	while ( getline(&buffer, &len, fp) != -1 ){
		char* key = strtok(buffer, ":");
		char* value = strtok(NULL, ":");

		/* ensure value exists */
		if ( !value ) value = empty;

		/* strip trailing newline */
		const size_t len = strlen(value);
		if ( value[len-1] == '\n' ) value[len-1] = 0;

		/* find in metatable */
		auto it = info->meta->find(std::string(key));
		if ( it != info->meta->end() ){
			it->second->set_string(this, value, 0);
		} else {
			fprintf(stderr, "%s: unknown key `%s' ignored.\n", filename.c_str(), key);
		}
	}
	free(buffer);
}

void Scene::meta_persist(){

}

const SceneFactory::Metadata& Scene::metadata() const {
	return *meta_definition;
}

void Scene::set_metadata(const SceneFactory::Metadata& metadata){
	meta_definition = &metadata;
}
