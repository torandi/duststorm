#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "enemy_template.hpp"
#include "enemy.hpp"
#include "render_object.hpp"
#include "config.hpp"
#include "utils.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

std::vector<EnemyTemplate> EnemyTemplate::templates;
std::map<std::string, EnemyAI*> EnemyTemplate::available_ais;
float EnemyTemplate::spawn_rate;

void EnemyTemplate::init(Config config) {
	available_ais["stare"] = new StaringAI();

	spawn_rate = config["spawn_rate"]->as_float();

	std::vector<ConfigEntry*> enemies = config["enemies"]->as_list();

	for(ConfigEntry * c : enemies) {
		templates.push_back(EnemyTemplate(c));
	}
}

EnemyTemplate::EnemyTemplate(const ConfigEntry * config) {
		model = new RenderObject(config->find("model")->as_string());
		min_scale = config->find("min_scale")->as_float();
		max_scale  =config->find("max_scale")->as_float();
		min_level = config->find("min_level")->as_float();
		hp_base = config->find("hp_base")->as_float();
		damage_base = config->find("damage_base")->as_float();
		random_movement = config->find("random_movement")->as_float();
		random_rotation = config->find("random_rotation")->as_float();
		spawn_cost = config->find("spawn_cost")->as_float();
		ai = available_ais[config->find("ai")->as_string()];
}

EnemyTemplate::~EnemyTemplate() {
	//delete model;
}

Enemy * EnemyTemplate::spawn(const glm::vec3 &position, float level_scaling) {
	Enemy * e = new Enemy(position, model, ai);
	float scale = min_scale + frand() * (max_scale - min_scale);
	e->set_scale(scale);
	e->radius = radius * scale;
	e->hp = hp_base * level_scaling;
	e->damage = damage_base * level_scaling;
	e->random_movement = random_movement;
	e->random_rotation = random_rotation;
	return e;
}

/* AIS */
void StaringAI::run(Enemy * enemy, float dt) const {
	//TODO!
}
