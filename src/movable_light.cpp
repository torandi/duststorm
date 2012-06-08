#define GLM_SWIZZLE

#include <glm/glm.hpp>

#include "movable_light.hpp"

MovableLight::MovableLight(Light * light) : MovableObject(light->position.xyz), data(light) { }

void MovableLight::update() {
	data->position.xyz = position_;
}	


