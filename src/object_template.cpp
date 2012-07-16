#include "object_template.hpp"
#include "game.hpp"

#define DOOR_RANGE 6.f
#define DOOR_TRY_TIMEOUT 2.f
#define PICKUP_RANGE 2.f

void ObjectTemplate::update(float dt) {
	if(ttl > 0.f) {
		ttl -= dt;
		if(ttl < 0.f) {
			destroyed = true;
		}
	}
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
	if(try_timeout > 0.f) return false;
	try_timeout = DOOR_TRY_TIMEOUT;
	if(obj->game.player->attr("blood") >= obj->game.area()->required_blood) {
		obj->game.change_area(area, entry_point);
		printf("Change area to %s : %s\n", area.c_str(), entry_point.c_str());
	} else {
		obj->game.play_sfx("moreblood");
	}
	return true;
}

bool Door::hit() {
	return false;
}

void Door::update(float dt) {
	if(try_timeout > 0.f) try_timeout -= dt;
	ObjectTemplate::update(dt);
}

void Pickup::update(float dt) {
	obj->absolute_rotate(glm::vec3(0, 1.f, 0), dt);
	ObjectTemplate::update(dt);
}

ObjectTemplate * Decoration::create(const YAML::Node &node, Game &game) {
	Decoration * d = new Decoration();
	d->obj = new Object2D(node, game);
	return d;
}

ObjectTemplate * Decoration::create(const std::string &vfx, Game &game) {
	Decoration * d = new Decoration();
	d->obj = new Object2D(vfx, game);
	return d;
}

ObjectTemplate * Pickup::create(const YAML::Node &node, Game &game) {
	Pickup * pickup = new Pickup();
	pickup->obj = new Object2D(node, game);
	pickup->attr = node["attribute"].as<std::string>();
	pickup->effect = node["effect"].as<int>();
	pickup->sfx = node["sfx"].as<std::string>();
	return pickup;
}

ObjectTemplate * Pickup::create(const std::string &vfx, const std::string &attr, int effect,float radius, const std::string & sfx, Game &game) {
	Pickup * pickup = new Pickup();
	pickup->obj = new Object2D(vfx, game);
	pickup->obj->hit_detection = true;
	pickup->obj->radius = radius;
	pickup->attr = attr;
	pickup->effect = effect;
	pickup->sfx = sfx;
	return pickup;
}

bool Pickup::click() {
	if( glm::length(obj->current_position - obj->game.player->current_position) < PICKUP_RANGE) {
		collision();
		return true;
	} else {
		return false;
	}
}

bool Pickup::collision() {
	obj->game.player->change_attr(attr,effect);
	obj->game.play_sfx(sfx);
	destroyed = true;
	return false;
}
