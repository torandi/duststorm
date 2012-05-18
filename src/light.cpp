#include "light.hpp"

#include <glm/glm.hpp>
#include <vector>

Light::Light(glm::vec3 _intensity) :
																MovableObject(), 
																intensity(_intensity) { 
	attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
}

Light::Light(glm::vec3 _intensity, glm::vec3 position) : 
																				MovableObject(position), 
																				intensity(_intensity) {
	attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
}

void Light::set_half_light_distance(float hld) {
	attenuation = 1.f/pow(hld,2);
}
