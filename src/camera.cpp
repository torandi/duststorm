#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <cmath>

Camera::Camera(float fov, float aspect, float near, float far) : 
	fov_(fov), aspect_(aspect), near_(near), far_(far) {
	projection_matrix_ = glm::perspective(fov_, aspect_, near_, far_);
}

const glm::vec3 Camera::look_at() const {
	return local_z()+position_;
}

void Camera::look_at(glm::vec3 position) {
	orientation_ = glm::fquat(1.f, 0.f, 0.f, 0.f); //Reset rotation
	glm::vec3 direction = position_ - position;
	glm::vec2 xz_projection = glm::vec2(direction.x,direction.z);

	float rotation = acosf(glm::dot(xz_projection, glm::vec2(0.f, 1.f)) * glm::length(xz_projection));
	absolute_rotate(glm::vec3(0.f, 1.f, 0.f), rotation);

	glm::vec3 lz = local_z();

	rotation = acosf(glm::dot(direction, lz) / (glm::length(direction) * glm::length(lz)));

	pitch(rotation);
}


const glm::mat4 Camera::view_matrix() const {
	return glm::lookAt(position(), look_at(), local_y());
}

const glm::mat4 Camera::projection_matrix() const { return projection_matrix_; }

const float Camera::fov() const { return fov_; }
const float Camera::aspect() const { return aspect_; }
const float Camera::near() const { return near_; }
const float Camera::far() const { return far_; }

void Camera::set_fov(float fov) {
	fov_ = fov;
	projection_matrix_ = glm::perspective(fov_, aspect_, near_, far_);
}
