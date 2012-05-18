#include "camera.h"

#include <glm/glm.hpp>

#include <cstdio>

const glm::vec3 Camera::look_at() const {
	return glm::vec3(rotation_matrix()*glm::vec4(0.0, 0.0, 1.0, 1.0))+position_;
}
const glm::vec3 Camera::up() const {
	return glm::vec3(rotation_matrix()*glm::vec4(0.0, 1.0, 0.0, 1.0));
}
