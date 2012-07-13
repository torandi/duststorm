#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "movable_light.hpp"
#include "object2d.hpp"

class Player : public Object2D {
	public:
		Player(const YAML::Node &node, Game &game_);
		glm::vec3 light_color;
		glm::vec3 light_offset;

		std::map<std::string, int> attributes;

		float weapon_radius[4];
		float weapon_damage[4];
		float click_radius;
};

#endif
