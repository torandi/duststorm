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

		YAML::Node node = YAML::LoadFile(str);
		free(str);

		if(node["type"].as<std::string>() == "model") {
			fprintf(verbose, "Create ModelVFX %s\n", name.c_str());
			VFX * r = new ModelVFX(node);
			loaded_vfx[name] = r;
			return r;
		} else if(node["type"].as<std::string>() == "particles") {
			fprintf(verbose, "Create ParticlesVFX %s\n", name.c_str());
			VFX * r = new ParticlesVFX(node);
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
	render_object_->set_position(node["offset"].as<glm::vec3>(glm::vec3(0.f)));
	if(node["rotation"]) {
		glm::vec3 axle = node["rotation"]["axle"].as<glm::vec3>();
		float angle = node["rotation"]["angle"].as<float>();
		render_object_->set_rotation(axle, angle);
	}
}

ModelVFX::~ModelVFX() {
	delete render_object_;
}

void ModelVFX::render(const glm::mat4 &matrix, const void * state) const {
	shaders[SHADER_NORMAL]->bind();
	render_object_->render(&matrix);
	Shader::unbind();
}

void * ModelVFX::update(float dt, void * state) { return state; }

void * ModelVFX::create_state() {
	return nullptr; 
}


ParticlesVFX::ParticlesVFX(const YAML::Node &node) {
	count = node["count"].as<int>();
	avg_spawn_rate = node["avg_spawn_rate"].as<float>();
	spawn_rate_var = node["spawn_rate_var"].as<float>();

	std::vector<std::string> texture_names;
	for(const YAML::Node &t : node["textures"]) {
		texture_names.push_back(t.as<std::string>());
	}

	config.birth_color = node["birth_color"].as<glm::vec4>();
	config.death_color = node["death_color"].as<glm::vec4>();
	config.spawn_position = glm::vec4(node["spawn_position"].as<glm::vec3>(glm::vec3(0.f)) , 0.f);
	config.spawn_area = node["spawn_area"].as<glm::vec4>();
	config.spawn_direction = glm::vec4(node["spawn_direction"].as<glm::vec3>(), 0.f);
	config.directional_speed = glm::vec4(node["directional_speed"].as<glm::vec3>(glm::vec3(0.f)), 0.f);
	config.directional_speed_var = glm::vec4(node["directional_speed_var"].as<glm::vec3>(glm::vec3(0.f)), 0.f);
	config.direction_var = glm::vec4(node["direction_var"].as<glm::vec3>(), 0.f);
	config.avg_spawn_speed= node["avg_spawn_speed"].as<float>();
	config.spawn_speed_var = node["spawn_speed_var"].as<float>();
	config.avg_ttl = node["avg_ttl"].as<float>();
	config.ttl_var = node["ttl_var"].as<float>();
	config.avg_scale = node["avg_scale"].as<float>();
	config.scale_var = node["scale_var"].as<float>();
	config.avg_acc = node["avg_acc"].as<float>(0.f);
	config.acc_var = node["acc_var"].as<float>(0.f);
	config.avg_scale_change = node["avg_scale_change"].as<float>();
	config.scale_change_var = node["scale_change_var"].as<float>();
	config.avg_rotation_speed = node["avg_rotation_speed"].as<float>();
	config.rotation_speed_var = node["rotation_speed_var"].as<float>();
	config.motion_rand = glm::vec4(node["motion_rand"].as<glm::vec3>(), 0);

	textures = TextureArray::from_filename(texture_names);


}

void ParticlesVFX::render(const glm::mat4 &matrix, const void * state) const {
	shaders[SHADER_PARTICLES]->bind();
	ParticleSystem* system = (ParticleSystem*) state;
	system->render(&matrix);
	Shader::unbind();
}

void * ParticlesVFX::update(float dt, void * state) {
	ParticleSystem* system = (ParticleSystem*) state;
	system->update(dt);
	return state;
}

void * ParticlesVFX::create_state() {
	ParticleSystem * system = new ParticleSystem(count, textures);
	system->avg_spawn_rate = avg_spawn_rate;
	system->spawn_rate_var = spawn_rate_var;
	system->config.birth_color = config.birth_color;
	system->config.death_color = config.death_color;
	system->config.motion_rand = config.motion_rand;
	system->config.spawn_direction = config.spawn_direction;
	system->config.direction_var = config.direction_var;
	system->config.spawn_position = config.spawn_position;
	system->config.spawn_area = config.spawn_area;
	system->config.directional_speed = config.directional_speed;
	system->config.directional_speed_var = config.directional_speed_var;
	system->config.avg_ttl = config.avg_ttl;
	system->config.ttl_var = config.ttl_var;
	system->config.avg_spawn_speed= config.avg_spawn_speed;
	system->config.spawn_speed_var = config.spawn_speed_var;
	system->config.avg_acc = config.avg_acc;
	system->config.acc_var = config.acc_var;
	system->config.avg_scale = config.avg_scale;
	system->config.scale_var = config.scale_var;
	system->config.avg_scale_change = config.avg_scale_change;
	system->config.scale_change_var = config.scale_change_var;
	system->config.avg_rotation_speed = config.avg_rotation_speed;
	system->config.rotation_speed_var = config.rotation_speed_var;

	system->update_config();
	return (void*) system;
}

void ParticlesVFX::set_spawn_rate(void * state, float spawn_rate, float spawn_rate_var) const {
	ParticleSystem* system = (ParticleSystem*) state;
	system->avg_spawn_rate = spawn_rate;
	system->spawn_rate_var = spawn_rate_var;
}
