#ifndef LIGHT_H
#define LIGHT_H

#include "platform.h"
#include <glm/glm.hpp>
#include <GL/glew.h>

struct Light {
	Light();

	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float is_directional;
	__ALIGNED__(glm::vec3 intensity, 16);
	__ALIGNED__(glm::vec3 position, 16);
	__ALIGNED__(glm::mat4 matrix, 16);
	__ALIGNED__(glm::vec2 shadowmap_scale, 16); // 1/width, 1/height of shadowmap resolution
	GLint shadowmap_index;
	float shadow_bias;
};

#endif
