#include "camera.hpp"
#include "utils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <cmath>
#include <cassert>

Camera::Camera(float fov, float aspect, float near, float far)
	: fov_(fov)
	, roll_(0.f)
	, aspect_(aspect)
	, near_(near)
	, far_(far) {

	projection_matrix_ = glm::perspective(fov_, aspect_, near_, far_);
}

const glm::vec3 Camera::look_at() const {
	return look_at_;
}

void Camera::look_at(const glm::vec3 &lookat) {
	look_at_ = lookat;

	recalculate();
}

/**
 * Recalculates rotation
 */
void Camera::recalculate() {
	orientation_ = glm::fquat(1.f, 0.f, 0.f, 0.f); //Reset rotation

	glm::vec3 direction = look_at_ - position_;
	glm::vec2 xz_projection = glm::vec2(direction.x,direction.z);
	float dot = glm::dot(xz_projection, glm::vec2(0.f, 1.f));

	float rotation = acosf(glm::clamp(dot / glm::length(xz_projection), -1.f, 1.f));
	absolute_rotate(glm::vec3(0.f, 1.f, 0.f), rotation*glm::sign(direction.x));

	glm::vec3 lz = local_z();

	rotation = acosf(glm::clamp(glm::dot(direction, lz) / (glm::length(direction) * glm::length(lz)), -1.f, 1.f));

	pitch(-rotation*glm::sign(direction.y));

	roll(roll_);
}


const glm::mat4 Camera::view_matrix() const {
	return glm::lookAt(position_, look_at_, local_y());
}

const glm::mat4 Camera::projection_matrix() const { return projection_matrix_; }

const float Camera::fov() const { return fov_; }
const float Camera::aspect() const { return aspect_; }
const float Camera::near() const { return near_; }
const float Camera::far() const { return far_; }
const float Camera::roll() const { return roll_; }
const glm::vec3 &Camera::position() const { return position_; }

void Camera::set_aspect(float aspect){
	aspect_ = aspect;
	projection_matrix_ = glm::perspective(fov_, aspect_, near_, far_);
}

void Camera::set_fov(float fov) {
	fov_ = fov;
	projection_matrix_ = glm::perspective(fov_, aspect_, near_, far_);
}

void Camera::set_roll(const float r) {
	roll(r-roll_);
	roll_ = r;
}

void Camera::roll(const float r) {
	roll_+=r;
	MovableObject::roll(r);
}

void Camera::relative_move(const glm::vec3 &move) {
	MovableObject::relative_move(move);
	recalculate();
}

void Camera::set_position(const glm::vec3 &pos) {
	MovableObject::set_position(pos);
	recalculate();
}
