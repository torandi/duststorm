#ifndef ENEMY_TEMPLATE_HPP
#define ENEMY_TEMPLATE_HPP

#include "movable_object.hpp"
#include "config.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

class EnemyAI;

class EnemyTemplate : public MovableObject {
	public:
		EnemyTemplate(const ConfigEntry * config);
		~EnemyTemplate();

		static void init(Config config);

		static std::vector<EnemyTemplate> templates;
		static float spawn_rate;

		Enemy * spawn(const glm::vec3 &position, float level_scaling);

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
};

class EnemyAI {
	public:
		virtual void run(Enemy * enemy, float dt) const = 0;
};

class StaringAI : public EnemyAI {
	public:
		virtual void run(Enemy * enemy, float dt) const;
};

#endif
