#include "camera.hpp"

#include <glm/glm.hpp>

#include <cstdio>
#include <cmath>

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
