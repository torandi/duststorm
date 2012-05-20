#include "camera.hpp"

#include "utils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <cmath>

#include <cassert>

Camera::Camera(float fov, float aspect, float near, float far) : 
	fov_(fov), aspect_(aspect), near_(near), far_(far) {
	projection_matrix_ = glm::perspective(fov_, aspect_, near_, far_);
}

const glm::vec3 Camera::look_at() const {
	return local_z()+position_;
}

void Camera::look_at(glm::vec3 lookat) {
	orientation_ = glm::fquat(1.f, 0.f, 0.f, 0.f); //Reset rotation
   
	glm::vec3 direction = lookat - position_;
	glm::vec2 xz_projection = glm::vec2(direction.x,direction.z);
   float dot = glm::dot(xz_projection, glm::vec2(0.f, 1.f));

	float rotation = acosf(glm::clamp(dot / glm::length(xz_projection), -1.f, 1.f));
	absolute_rotate(glm::vec3(0.f, 1.f, 0.f), rotation*glm::sign(direction.x));
   

   
	glm::vec3 lz = local_z();

	rotation = acosf(glm::clamp(glm::dot(direction, lz) / (glm::length(direction) * glm::length(lz)), -1.f, 1.f));

	pitch(-rotation*glm::sign(direction.y));
   
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
