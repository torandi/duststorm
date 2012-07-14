#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "movable_light.hpp"
#include "object2d.hpp"

class Player : public Object2D {
	public:
		Player(const YAML::Node &node, Game &game_);
		glm::vec3 light_color;
		glm::vec3 light_offset;

		int &attr(const std::string attr);
		
		std::map<std::string, int> attributes;

		virtual void update(float dt);
		virtual void render();
		void swing();

		glm::vec3 center3() const;

		float weapon_radius[4];
		float weapon_damage[4];
		float click_radius;
		float swing_state;
		RenderObject chainsaw;
};

#endif
