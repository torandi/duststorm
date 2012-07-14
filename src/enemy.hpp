#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "object2d.hpp"
#include "yaml-helper.hpp"

#include <list>

class Game;

class Enemy : public Object2D {
	public:
		Enemy(const YAML::Node &node, Game &game);

		virtual void update(float dt);

		float life;
		float max_life;
		float life_regen;
		bool dead;
		float attack_radius;
		float trigger_radius;
		float damage;
		std::string attack_sfx;
		std::string name;
		bool highlighted;

		float attack_cooldown;
		float attack_repeat;

		struct drop_t {
			std::string name;
			glm::ivec2 rate;
		};

		std::list<drop_t> drops;
};

typedef Enemy* (enemy_create) (const YAML::Node &node, Game &game);

class BasicAI : public Enemy {
	public: 
		BasicAI(const YAML::Node &node, Game &game);

		virtual void update(float dt);
		virtual void attack();

		static Enemy * create(const YAML::Node &node, Game &game);
};

#endif
