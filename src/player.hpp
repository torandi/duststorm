#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "movable_light.hpp"
#include "movable_object.hpp"
#include "yaml-helper.hpp"
#include "vfx.hpp"

class Player : public MovableObject {
	public:
		Player(const YAML::Node &node);
		MovableLight light;

		void render() const;
		void update(float dt);
	private:
		VFX * vfx;
		void * vfx_state;
};

#endif
