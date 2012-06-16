#ifndef MOVABLE_LIGHT_H
#define MOVABLE_LIGHT_H

#include "movable_object.hpp"
#include "light.hpp"

class MovableLight : public MovableObject {
	public:
		MovableLight(Light * light);

		void update(); //Must be called to update position in light

		float &constant_attenuation;
		float &linear_attenuation;
		float &quadratic_attenuation;
		Light::light_type_t& type;
		glm::vec3 &intensity;

	private:
		Light * data;
};

#endif
