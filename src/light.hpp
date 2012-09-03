#ifndef LIGHT_H
#define LIGHT_H

#include "platform.h"
#include <glm/glm.hpp>

struct Light {
	Light();

	enum light_type_t: int {
		DIRECTIONAL_LIGHT = 0, //No attenuation
		POINT_LIGHT = 1
	};

	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	light_type_t type;
	glm::vec3 intensity;
	__ALIGNED__(glm::vec3 position, 16);
};

#endif
