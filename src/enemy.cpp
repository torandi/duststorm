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
	, fly_in(2.0)
	{
}

void Enemy::update(float dt) {
	if(fly_in > 0.25f) {
		absolute_move(glm::vec3(0.f, dt * 2.0, 0.f));
		fly_in -= dt;
	} else if(fly_in > 0.f) {
		absolute_move(glm::vec3(0.f, dt * 1.f * cos((fly_in/0.25) * M_PI), 0.f));
		fly_in -= dt;
	}
	ai->run(this, dt);
}

void Enemy::render() const {
	model->render(matrix());
}
