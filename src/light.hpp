#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "movable_object.h"

#define HALF_LIGHT_DISTANCE 1.5f

class Light : public MovableObject {
public:
	enum light_type_t {
		DIRECTIONAL_LIGHT, //No attenuation
		POINT_LIGHT
	} light_type;

	glm::vec3 intensity;

	Light(glm::vec3 _intensity, light_type_t lt );
	Light(glm::vec3 _intensity, glm::vec3 position, light_type_t lt);
	virtual ~Light() {};

	struct shader_light_t {
		float attenuation;
		glm::vec4 intensity;
		glm::vec4 position;
	};

private:
	shader_light_t shader_light_;

public:
	const shader_light_t &shader_light() const;

	void set_half_light_distance(float hld);
#endif
