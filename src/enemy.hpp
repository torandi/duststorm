#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "object2d.hpp"
#include "yaml-helper.hpp"

class Game;

class Enemy : public Object2D {
	public:
		Enemy(const YAML::Node &node, Game &game);

		virtual void update(float dt);

		float life;
		float max_life;
		float life_regen;
		bool dead;
		std::string name;
		bool highlighted;
};

typedef Enemy* (enemy_create) (const YAML::Node &node, Game &game);

class BasicAI : public Enemy {
	public: 
		BasicAI(const YAML::Node &node, Game &game);

		virtual void update(float dt);

		static Enemy * create(const YAML::Node &node, Game &game);
};

#endif
