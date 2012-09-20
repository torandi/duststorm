#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "enemy.hpp"
#include "config.hpp"
#include "render_object.hpp"
#include <glm/glm.hpp>

Enemy::Enemy(const glm::vec3 &position, const RenderObject * model_, const EnemyAI * ai_) : 
		MovableObject(position)
	, model(model_)
	, ai(ai_)
	{
	
}

void Enemy::update(float dt) {
	ai->run(this, dt);
}

void Enemy::render() {
	model->render(matrix());
}
