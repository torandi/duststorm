#include "object_template.hpp"
#include "game.hpp"

#define DOOR_RANGE 6.f
#define PICKUP_RANGE 2.f

void ObjectTemplate::update(float dt) {
	obj->update(dt);
}

void ObjectTemplate::render() const {
	obj->render();
}


ObjectTemplate::~ObjectTemplate() {
	delete obj;
}

ObjectTemplate * Door::create(const YAML::Node &node, Game &game) {
	Door * door = new Door();
	door->obj = new Object2D(node, game);
	door->area = node["area"].as<std::string>();
	door->entry_point = node["entry_point"].as<std::string>();
	return door;
}

bool Door::click() {
	//if( obj->game.player->hit(obj->current_position, DOOR_RANGE, true) ) {
		return collision();
	/*} else {
		return false;
	}*/
}

bool Door::collision() {
	obj->game.change_area(area, entry_point);
	printf("Change area to %s : %s\n", area.c_str(), entry_point.c_str());
	return true;
}

bool Door::hit() {
	return false;
}

ObjectTemplate * Decoration::create(const YAML::Node &node, Game &game) {
	Decoration * d = new Decoration();
	d->obj = new Object2D(node, game);
	return d;
}

ObjectTemplate * Pickup::create(const YAML::Node &node, Game &game) {
	Pickup * pickup = new Pickup();
	pickup->obj = new Object2D(node, game);
	pickup->attr = node["attribute"].as<std::string>();
	pickup->effect = node["effect"].as<int>();
	return pickup;
}

ObjectTemplate * Pickup::create(const std::string &vfx, const std::string &attr, int effect, Game &game) {
	Pickup * pickup = new Pickup();
	pickup->obj = new Object2D(vfx, game);
	pickup->attr = attr;
	pickup->effect = effect;
	return pickup;
}

bool Pickup::click() {
	if( glm::length(obj->current_position - obj->game.player->current_position) < PICKUP_RANGE) {
		return collision();
	} else {
		return false;
	}
}

bool Pickup::collision() {
	destroyed = true;
	obj->game.player->attributes[attr] += effect;
	//Play sound
	return true;
}
