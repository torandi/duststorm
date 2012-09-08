#ifndef LIGHT_H
#define LIGHT_H

#include "platform.h"
#include <glm/glm.hpp>

struct Light {
	Light();

	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float is_directional;
	glm::vec3 intensity;
	__ALIGNED__(glm::vec3 position, 16);
};

#endif
