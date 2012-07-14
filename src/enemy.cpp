#include "enemy.hpp"
#include "game.hpp"

Enemy::Enemy(const YAML::Node &node, Game &game) : Object2D(node, game), dead(false), highlighted(false) {
	max_life = life = node["life"].as<float>();
	life_regen = node["life_regen"].as<float>(0.f);
	name = node["name"].as<std::string>();
	attack_sfx = node["attack_sfx"].as<std::string>();
	attack_radius = node["attack_radius"].as<float>();
	trigger_radius = node["trigger_radius"].as<float>();
	damage = node["damage"].as<float>();

	for(auto it = node["drops"].begin(); it != node["drops"].end(); ++it) {
		drop_t d;
		d.name = it->first.as<std::string>();
		d.rate = it->second.as<glm::ivec2>();
		drops.push_back(d);
	}

	hit_detection = true;
}

void Enemy::update(float dt) {
	if(life < max_life) {
		life += life_regen*dt;
		if(life > max_life)
			life = max_life;
	}
	Object2D::update(dt);
}

Enemy * BasicAI::create(const YAML::Node &node, Game &game) {
	return new BasicAI(node, game);
}

BasicAI::BasicAI(const YAML::Node &node, Game &game) : Enemy(node, game) {
	
}

void BasicAI::update(float dt) {
	//Do ai
}
