#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "player.hpp"
#include "shader.hpp"
#include "render_object.hpp"
#include "path.hpp"

static const float tilt_position_diff = 0.1f;

Player::Player() {
	model = new RenderObject("kanon5.obj");
	model->yaw(-M_PI/2.f);
	model->set_position(glm::vec3(0.f, 0.8f, 0.f));
	shader = Shader::create_shader("normal");
}

Player::~Player() {
	delete model;
}

void Player::update_position(const Path * path, float pos) {
	path_position_ = pos;
	position_ = path->at(pos);
	const glm::vec3 direction = glm::normalize(path->at(pos + tilt_position_diff) - position_);
	//const glm::vec3 side = glm::normalize(glm::cross(direction, initial_normal)); //points right
	//const glm::vec3 normal = glm::normalize(glm::cross(side, direction));

	orientation_ = glm::fquat(1.f, 0.f, 0.f, 0.f); //Reset rotation

	glm::vec2 xz_projection = glm::vec2(direction.x,direction.z);
	float dot = glm::dot(xz_projection, glm::vec2(0.f, 1.f));

	float rotation = acosf(glm::clamp(dot / glm::length(xz_projection), -1.f, 1.f));
	MovableObject::absolute_rotate(glm::vec3(0.f, 1.f, 0.f), rotation*glm::sign(direction.x));

	glm::vec3 lz = local_z();

	rotation = acosf(glm::clamp(glm::dot(direction, lz) / (glm::length(direction) * glm::length(lz)), -1.f, 1.f));

	MovableObject::relative_rotate(glm::vec3(1.f, 0.f, 0.f),-rotation*glm::sign(direction.y));

	//MovableObject::relative_rotate(glm::vec3(0.f, 0.f, 1.f), roll_);
	rotation_matrix_dirty_ = true;
	translation_matrix_dirty_ = true;
}

void Player::render_geometry(const glm::mat4 &m) {
	//Todo: handle rotation of model groups
	model->render(matrix() * m);
}

void Player::render(const glm::mat4 &m) {
	shader->bind();
	render_geometry(m);
}

const float Player::path_position() const { return path_position_; }
