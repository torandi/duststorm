#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "enemy_template.hpp"
#include "enemy.hpp"
#include "render_object.hpp"
#include "config.hpp"
#include "utils.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>

std::vector<EnemyTemplate> EnemyTemplate::templates;
std::map<std::string, EnemyAI*> EnemyTemplate::available_ais;
float EnemyTemplate::spawn_rate;
float EnemyTemplate::min_spawn_cost = FLT_MAX;
int EnemyTemplate::max_num_enemies;

void EnemyTemplate::init(Config config, const Game * game) {
	available_ais["stare"] = new StaringAI(game);

	spawn_rate = config["spawn_rate"]->as_float();
	max_num_enemies = config["max_num_enemies"]->as_int();

	std::vector<ConfigEntry*> enemies = config["enemies"]->as_list();

	for(ConfigEntry * c : enemies) {
		templates.push_back(EnemyTemplate(c));
	}
}

EnemyTemplate::EnemyTemplate(const ConfigEntry * config) {
		model = new RenderObject(config->find("model")->as_string(), true);
		model->set_position(glm::vec3(0.0, 0.0, 0.0));
		model->set_rotation(glm::vec3(0.f, 1.f, 0.f), -90); //Hack because I'm lazy
		min_scale = config->find("min_scale")->as_float();
		max_scale  =config->find("max_scale")->as_float();
		min_level = config->find("min_level")->as_float();
		hp_base = config->find("hp_base")->as_float();
		damage_base = config->find("damage_base")->as_float();
		random_movement = config->find("random_movement")->as_float();
		random_rotation = config->find("random_rotation")->as_float();
		spawn_cost = config->find("spawn_cost")->as_float();
		ai = available_ais[config->find("ai")->as_string()];
		if(spawn_cost < min_spawn_cost) min_spawn_cost = spawn_cost;
}

EnemyTemplate::~EnemyTemplate() {
}

Enemy * EnemyTemplate::spawn(const glm::vec3 &position, float path_position, float level_scaling) {
	float scale = min_scale + frand() * (max_scale - min_scale);
	Enemy * e = new Enemy(position - glm::vec3(0.0, scale, 0.0), model, ai);
	e->set_scale(scale);
	e->radius = radius * scale;
	e->hp = hp_base * level_scaling;
	e->damage = damage_base * level_scaling;
	e->random_movement = random_movement;
	e->random_rotation = random_rotation;
	e->path_position = path_position;
	return e;
}

EnemyAI::EnemyAI(const Game * g) : game(g) {};
StaringAI::StaringAI(const Game * game) : EnemyAI(game) {};

/* AIS */
void StaringAI::run(Enemy * enemy, float dt) const {
	const glm::vec3 direction = glm::normalize(game->get_player().position() - enemy->position());

	glm::vec2 xz_projection = glm::vec2(direction.x,direction.z);

	float rotation = acosf(glm::clamp(xz_projection.y / glm::length(xz_projection), -1.f, 1.f));
	enemy->set_rotation(glm::vec3(0.f, 1.f, 0.f), glm::degrees(rotation*glm::sign(direction.x)));
}
