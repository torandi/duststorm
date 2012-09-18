#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glm/gtx/string_cast.hpp>

#include "player.hpp"
#include "shader.hpp"
#include "render_object.hpp"
#include "path.hpp"
#include "globals.hpp"

static const float pitch_position_diff = 0.1f;

Player::Player() {
	cart = new RenderObject("canon/cart.obj", true);
	holder = new RenderObject("canon/holder.obj");
	gun = new RenderObject("canon/gun.obj");
	cart->yaw(-M_PI/2.f);
	cart->set_position(glm::vec3(0.f, 0.6f, 0.f));
	shader = Shader::create_shader("normal");
	canon_pitch.set_rotation(glm::vec3(1.0, 0.0, 0.0), 0.f);
	canon_yaw.set_rotation(glm::vec3(1.0, 0.0, 0.0), 0.f);
}

Player::~Player() {
	delete cart;
	delete holder;
	delete gun;
}

void Player::update_position(const Path * path, float pos) {
	path_position_ = pos;
	position_ = path->at(pos);
	const glm::vec3 direction = glm::normalize(path->at(pos + pitch_position_diff) - position_);
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

void Player::render_geometry(const glm::mat4 &m_) {
	glm::mat4 m = m_ * matrix();
	cart->render(m);
	m = m * cart->matrix() * canon_yaw.rotation_matrix();
	holder->render(m);
	m = m * holder->matrix() * canon_pitch.rotation_matrix();
	gun->render(m);

}

void Player::render(const glm::mat4 &m) {
	shader->bind();
	render_geometry(m);
}

const float Player::path_position() const { return path_position_; }

glm::vec3 Player::direction() const {
	return glm::normalize(local_z());
}

glm::mat4 Player::aim_matrix() const {
	return rotation_matrix() * canon_yaw.rotation_matrix() * canon_pitch.rotation_matrix();
}

void Player::set_canon_yaw(float angle) {
	canon_yaw.set_rotation(glm::vec3(0.f, 1.0, 0.0), angle);
}

void Player::set_canon_pitch(float angle) {
	angle = glm::clamp(angle, 0.f, 90.f);
	canon_pitch.set_rotation(glm::vec3(0.f, 0.f, 1.f), angle);
}
