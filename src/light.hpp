#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "movable_object.hpp"

#define HALF_LIGHT_DISTANCE 1.5f

class Light : public MovableObject {
public:
	glm::vec3 intensity;
	float attenuation;

	Light(glm::vec3 _intensity);
	Light(glm::vec3 _intensity, glm::vec3 position);
	virtual ~Light() {};

	void set_half_light_distance(float hld);

};
#endif
