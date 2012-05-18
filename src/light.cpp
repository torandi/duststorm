#include "light.h"

#include <glm/glm.hpp>
#include <vector>

Light::Light(glm::vec3 _intensity, light_type_t lt ) :
																MovableObject(), 
																light_type(lt) ,
																intensity(_intensity) { 
	shader_light_.attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
}

Light::Light(glm::vec3 _intensity, glm::vec3 position, light_type_t lt) : 
																				MovableObject(position), 
																				light_type(lt) , 
																				intensity(_intensity) {
	shader_light_.attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
}

const Light::shader_light_t &Light::shader_light() const {
	shader_light_.intensity.x = intensity.x;
	shader_light_.intensity.y = intensity.y;
	shader_light_.intensity.z = intensity.z;
	shader_light_.position.x = position().x;
	shader_light_.position.y = position().y;
	shader_light_.position.z = position().z;
	shader_light_.position.w = 1.0f;
	//the w component turns on and of light attenuation
	switch(light_type) {
		case DIRECTIONAL_LIGHT:
			shader_light_.position.w = 0.0;
			break;
		case POINT_LIGHT:
			shader_light_.position.w = 1.0;
			break;
	}
	return shader_light_;
}

void Light::set_half_light_distance(float hld) {
	shader_light_.attenuation = 1.f/pow(hld,2);
}

const shader_light_t &shader_light() const {
	return shader_light_;
}
