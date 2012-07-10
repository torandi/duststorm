#include "vfx.hpp"
#include "data.hpp"
#include "globals.hpp"

std::map<std::string, VFX*> VFX::loaded_vfx;

VFX * VFX::get_vfx(const std::string &name) {
	auto it = loaded_vfx.find(name);
	if(it != loaded_vfx.end()) {
		return it->second;
	} else {
		char * str;
		if(asprintf(&str, PATH_BASE "/game/vfx/%s.yaml", name.c_str()) == -1) abort();

		Data * src = Data::open(str);
		free(str);
		YAML::Node node = YAML::Load((char*)(src->data()));
		delete src;

		if(node["type"].as<std::string>() == "model") {
			VFX * r = new ModelVFX(node);
			loaded_vfx[name] = r;
			return r;
		}
		printf("Invalid vfx type\n");
		abort();
	}
}

ModelVFX::ModelVFX(const YAML::Node &node) {
	render_object_ = new RenderObject(node["model"].as<std::string>());
	render_object_->set_scale(node["scale"].as<glm::vec3>(glm::vec3(1.f)));
}

ModelVFX::~ModelVFX() {
	delete render_object_;
}

void ModelVFX::render(const glm::mat4 &matrix) const {
	render_object_->render(&matrix);
}

void * ModelVFX::update(float dt, void * state) { return state; }

void * ModelVFX::create_state() {
	return nullptr; //dont use state
}
