#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "enemy.hpp"
#include "config.hpp"
#include "render_object.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Enemy::Enemy(const glm::vec3 &position, const RenderObject * model_, const EnemyAI * ai_) : 
		MovableObject(position)
	, model(model_)
	, ai(ai_)
	, fly_in(2.0)
	{
		hp_shader = Shader::create_shader("health");
		enemy_shader = Shader::create_shader("normal");
}

void Enemy::update(float dt) {
	if(fly_in > 0.25f) {
		absolute_move(glm::vec3(0.f, dt * 2.0, 0.f));
		fly_in -= dt;
	} else if(fly_in > 0.f) {
		absolute_move(glm::vec3(0.f, dt * 1.f * cos((fly_in/0.25) * M_PI), 0.f));
		fly_in -= dt;
	}

	//Basic woobling (Later, maybe)
	

	ai->run(this, dt);
}

void Enemy::render_geometry() const {
	model->render(matrix());
}

void Enemy::render() const {
	enemy_shader->bind();
	render_geometry();
	hp_shader->bind();

	Shader::upload_model_matrix(matrix());

	float life = glm::clamp(hp/initial_hp, 0.f, 1.f);
	float scale = life * scale_.x;
	Shader::push_vertex_attribs(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, &life);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, &scale);

	glDrawArrays(GL_POINTS, 0, 1);

	Shader::pop_vertex_attribs();

}

void Enemy::set_hp(float _hp) {
	initial_hp = _hp;
	hp = _hp;
}
