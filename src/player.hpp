#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "movable_light.hpp"
#include "movable_object.hpp"
#include "yaml-helper.hpp"
#include "vfx.hpp"

class Game;

class Player : public MovableObject {
	public:
		Player(const YAML::Node &node, Game &game_);
		glm::vec3 light_color;
		glm::vec3 light_offset;
		glm::vec2 target;
		glm::vec2 current_position;

		void move_to(const glm::vec2 &pos);

		void render() const;
		void update(float dt);
	private:
		void update_position();
		VFX * vfx;
		void * vfx_state;
		Game &game;

		float speed;
};

#endif
