#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "movable_light.hpp"
#include "movable_object.hpp"
#include "yaml-helper.hpp"

class Player : MovableObject {
	public:
		Player(const YAML::Node &node);
		MovableLight light;
};

#endif
