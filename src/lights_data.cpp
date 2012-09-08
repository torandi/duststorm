#include "lights_data.hpp"
#include "shader.hpp"

LightsData::LightsData() {
	int index=0;
	for(Light &light : data_.lights) {
		light.shadowmap_index = index;
		lights[index++] = new MovableLight(&light);
	}
	data_.num_lights = 0;
	data_.ambient_intensity = glm::vec3(0.1f);
}

LightsData::~LightsData() {
	for(MovableLight * light : lights) {
		delete light;
	}
}

const Shader::lights_data_t &LightsData::shader_data() {
	for(int i=0; i < data_.num_lights; ++i) {
		lights[i]->update();
	}
	return data_;
}
