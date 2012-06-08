#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

struct Light{
	
	Light();

	enum light_type_t {
		DIRECTIONAL_LIGHT = 0, //No attenuation
		POINT_LIGHT = 1
	} light_type;

	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	int type; //light_type_t
	glm::vec4 intensity;
	glm::vec4 position;
};
#endif
