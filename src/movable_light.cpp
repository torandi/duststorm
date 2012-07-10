#define GLM_SWIZZLE

#include <glm/glm.hpp>

#include "movable_light.hpp"

MovableLight::MovableLight(Light * light) : 
		MovableObject(light->position.xyz)
	, data(light)
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, type(data->type)
	, intensity(data->intensity)
	{ }

MovableLight::MovableLight() :
	  data(new Light())
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, type(data->type)
	, intensity(data->intensity) {}

MovableLight::MovableLight(const MovableLight &ml) : MovableObject(ml.position())
	, data(ml.data)
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, type(data->type)
	, intensity(data->intensity) {}

void MovableLight::update() {
	data->position = position_;
}	

