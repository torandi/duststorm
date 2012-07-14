#include "object2d.hpp"
#include "game.hpp"
#include "utils.hpp"

#define MIN_MOVE 0.1f

Object2D::Object2D(const std::string &vfx_name, Game &game_) : 
		target(0.f)
	, current_position(0.f)
	, speed(1.f)
	, radius(0.f)
	, hit_detection(false)
	, current_rotation(0.f)
	,	game(game_) 
	, base_rotation(0.f)
	, center_offset(0.f)
	{

	vfx = VFX::get_vfx(vfx_name);
	vfx_state = vfx->create_state();
}

Object2D::Object2D(const YAML::Node &node, Game &game_) : 
		target(0.f)
	, current_position(0.f)
	, speed(1.f)
	, radius(0.f)
	, current_rotation(0.f)
	, hit_detection(false)
	,	game(game_) {
	speed = node["speed"].as<float>(1.f);
	radius = node["radius"].as<float>(0.f);
	hit_detection = node["enable_collision"].as<bool>(false);

	set_scale(node["scale"].as<glm::vec3>(glm::vec3(1.f)));
	base_rotation = node["rotation"].as<float>(0.f);

	center_offset = node["center_offset"].as<glm::vec2>(glm::vec2(0.f));
	height = node["height"].as<float>(0.f);

	set_rotation(0.f);

	vfx = VFX::get_vfx(node["vfx"].as<std::string>());
	vfx_state = vfx->create_state();
}

glm::vec2 Object2D::center() const {
	return current_position + center_offset;
}


void Object2D::rotate(float angle) {
	current_rotation += angle;
	absolute_rotate(glm::vec3(0.f, 1.f, 0.f), angle);
}

void Object2D::set_rotation(float angle) {
	current_rotation = angle;
	MovableObject::set_rotation(glm::vec3(0.f, 1.f, 0.f), base_rotation + angle);
}

void Object2D::face(const glm::vec2 &pos) {
	glm::vec2 dir = glm::normalize(center() - pos);
	float rot = radians_to_degrees(atan2(dir.y, dir.x));
	set_rotation(-rot);
}

void Object2D::face(const Object2D * obj) {
	face(obj->center());
}

void Object2D::move_to(const glm::vec2 &pos) {
	set_position(glm::vec3(pos.x, game.area()->height_at(pos)+height, pos.y));
	target = pos;
	current_position = pos - center_offset;
}

void Object2D::render() const {
	vfx->render(matrix(), vfx_state);
}

bool Object2D::hit(const Object2D * other) const {
	if(!other->hit_detection)
		return false;
	else
		return hit(other->center(), other->radius, false);
}

bool Object2D::hit(const glm::vec2 &pos, float r, bool ignore_flag) const {
	if(!ignore_flag && !hit_detection)
		return false;
	else {
		glm::vec2 dv = (center() - pos);
		float dist = glm::length(dv);
		if(dist <  (r + radius)) {
			return true;
		}	else {
			return false;
		}
	}
}

void Object2D::update(float dt) {
	vfx_state = vfx->update(dt, vfx_state);
	glm::vec2 prev = current_position;
	if(target != center()) {
		glm::vec2 diff = target - center();
		if(glm::length(diff) < MIN_MOVE) {
			target = center();
		} else if(diff.length() < speed * dt) {
			current_position = target - center_offset;
		} else {
			current_position += glm::normalize(diff) * speed * dt;
		}
		if(game.area()->collision_at(center())) {
			printf("Collision!\n");
			current_position = prev;
			target = prev;
		}
		update_position();
	}
}

void Object2D::update_position() {
	set_position(glm::vec3(current_position.x, game.area()->height_at(current_position)+height, current_position.y));
}
