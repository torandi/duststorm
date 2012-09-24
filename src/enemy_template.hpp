#ifndef ENEMY_TEMPLATE_HPP
#define ENEMY_TEMPLATE_HPP

#include "movable_object.hpp"
#include "config.hpp"
#include "game.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

class EnemyAI;

class EnemyTemplate : public MovableObject {
	public:
		EnemyTemplate(const ConfigEntry * config);
		~EnemyTemplate();

		static void init(Config config, const Game * game);

		static std::vector<EnemyTemplate> templates;
		static float spawn_rate;
		static float min_spawn_cost;
		static int max_num_enemies;

		Enemy * spawn(const glm::vec3 &position, float path_position, float level_scaling);

		float min_level; //Required level of player to spawn this
		float spawn_cost; //The cost of spawning this enemy (drawn from spawn_rate)
	private:

		static std::map<std::string, EnemyAI*> available_ais;

		float min_scale, max_scale;
		float hp_base;
		float damage_base;
		float random_movement;
		float random_rotation;
		float radius;
		RenderObject * model;
		const EnemyAI * ai;
		const Game * game;
};

class EnemyAI {
	public:
		EnemyAI(const Game * g);
		virtual void run(Enemy * enemy, float dt) const = 0;
	protected:
		const Game * game;
};

class StaringAI : public EnemyAI {
	public:
		StaringAI(const Game * game);
		virtual void run(Enemy * enemy, float dt) const;
};

#endif
