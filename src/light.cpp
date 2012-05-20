#include "light.hpp"

#include <glm/glm.hpp>
#include <vector>

Light::Light(Light::light_type_t type, glm::vec3 _intensity) : MovableObject() {
   attributes_.intensity = _intensity;
	attributes_.attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
   attributes_.position = position();
   attributes_.type = type;
}

Light::Light(Light::light_type_t type, glm::vec3 _intensity, glm::vec3 pos) : MovableObject(pos) {
   attributes_.intensity = _intensity;
	attributes_.attenuation = 1.f/pow(HALF_LIGHT_DISTANCE,2);
   attributes_.position = position();
   attributes_.type = type;
}

void Light::set_half_light_distance(float hld) {
	attributes_.attenuation = 1.f/pow(hld,2);
}

const float &Light::attenuation() const { return attributes_.attenuation; }

const glm::vec3 &Light::intensity() const { return attributes_.intensity; }

const Light::shader_light_t &Light::shader_light() { return attributes_; }

void Light::set_position(const glm::vec3 &pos) {
   MovableObject::set_position(pos);
   attributes_.position = position();
}

const Light::light_type_t Light::type() const { (light_type_t) attributes_.type; }

void Light::set_type(Light::light_type_t type) {
   attributes_.type = type;
}
