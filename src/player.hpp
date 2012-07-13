#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "movable_light.hpp"
#include "object2d.hpp"

class Player : public Object2D {
	public:
		Player(const YAML::Node &node, Game &game_);
		glm::vec3 light_color;
		glm::vec3 light_offset;
};

#endif
