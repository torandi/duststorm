#include "lights_data.hpp"
#include "shader.hpp"

LightsData::LightsData()
	: lights({
		&data_.lights[0],
		&data_.lights[1],
		&data_.lights[2],
		&data_.lights[3]
	}){

	data_.num_lights = 0;
	data_.ambient_intensity = glm::vec3(0.1f);
}

LightsData::~LightsData() {
}

const Shader::lights_data_t &LightsData::shader_data() {
	for(int i=0; i < data_.num_lights; ++i) {
		lights[i].update();
	}
	return data_;
}
