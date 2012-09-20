#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "movable_object.hpp"
#include "enemy_template.hpp"
#include "config.hpp"
#include <glm/glm.hpp>

class Enemy : public MovableObject {
	public:
		Enemy(const glm::vec3 &position, const RenderObject * model_, const EnemyAI * ai_);

		void update(float dt);
		void render();

		float hp;
		float damage;
		float random_movement;
		float random_rotation;
		float radius;

	private:
		const RenderObject * model;
		const EnemyAI * ai;
};

#endif
