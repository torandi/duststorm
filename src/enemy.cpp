#include "enemy.hpp"
#include "game.hpp"
#include "utils.hpp"

Enemy::Enemy(const YAML::Node &node, Game &game) : Object2D(node, game), dead(false), highlighted(false) {
	max_life = life = node["life"].as<float>();
	life_regen = node["life_regen"].as<float>(0.f);
	name = node["name"].as<std::string>();
	attack_sfx = node["attack_sfx"].as<std::string>();
	attack_radius = node["attack_radius"].as<float>();
	trigger_radius = node["trigger_radius"].as<float>();
	damage = node["damage"].as<float>();
	attack_repeat = node["attack_repeat"].as<float>();
	attack_cooldown = -0.1f;

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
	Object2D::update(dt);
	if(distance(game.player) < trigger_radius) {
		glm::vec3 lz = game.player->local_z();
		target = game.player->center() -  glm::vec2(lz.x, lz.z) * game.player->radius + glm::vec2(frand()*2.f, frand()*2.f);
		face(game.player->center());
		if(attack_cooldown <= 0.f && distance(game.player) < (attack_radius + radius + game.player->radius)) {
			attack();
		} else if(attack_cooldown > 0.f){
			attack_cooldown -= dt;
		}
	}
}

void BasicAI::attack() {
	if(rand() % 10 == 0) game.play_sfx(attack_sfx);
	game.player->damage(damage);
	attack_cooldown = attack_repeat;
}
