#ifndef LIGHTS_DATA_H
#define LIGHTS_DATA_H

#include "movable_light.hpp"
#include "shader.hpp"
#include <glm/glm.hpp>

class LightsData {
public:
	LightsData();
	~LightsData();
	
	MovableLight *lights[MAX_NUM_LIGHTS];

	glm::vec3 &ambient_intensity() { return data_.ambient_intensity; };
	int &num_lights() { return data_.num_lights; };

	const Shader::lights_data_t &shader_data();

private:
	Shader::lights_data_t data_;
};

#endif
