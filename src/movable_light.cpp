#include <glm/glm.hpp>

#include "movable_light.hpp"
#include "globals.hpp"

MovableLight::MovableLight(Light * light) : 
		MovableObject(light->position)
	, data(light)
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity)
	, type(MovableLight::DIRECTIONAL_LIGHT)
	{ 
		shadowmap = Texture2D::from_filename(PATH_BASE "/textures/white.png");
		update();
	}

MovableLight::MovableLight() :
	  data(new Light())
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) {}

MovableLight::MovableLight(const MovableLight &ml) : MovableObject(ml.position())
	, data(ml.data)
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) {}

void MovableLight::update() {
	data->position = position_;
	data->is_directional = (type == DIRECTIONAL_LIGHT);
}	

