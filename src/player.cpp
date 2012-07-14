#include "player.hpp"
#include "globals.hpp"
#include "game.hpp"

Player::Player(const YAML::Node &node, Game &game_) : Object2D(node, game_)
	, swing_state(-1.f)
	,	chainsaw("chainsaw.obj")
	{
	light_color = node["light"].as<glm::vec3>(glm::vec3(0.8f));
	light_offset = node["light_offset"].as<glm::vec3>(glm::vec3(1.0)) + glm::vec3(center_offset.x, 0.f, center_offset.y);

	attributes["life"] = node["life"].as<float>();
	attributes_max["life"] = attributes["life"];

	score = 0;

	const YAML::Node &n = node["weapons"];
	for(int i=0; i<4; ++i) {
		weapon_radius[i] = n[i]["radius"].as<float>();
		weapon_damage[i] = n[i]["damage"].as<float>();
	}

	click_radius = node["click_radius"].as<float>();
	hit_detection = true;
	dead = false;

	//Lets do teh ugly hack:
	for(auto &m : chainsaw.materials) {
		m.two_sided = true;
	}
}

void Player::damage(float dmg) {
	change_attr("life", -dmg);
	if(attr("life") <= 0) {
		dead = true;
	}
}

int Player::attr(const std::string &attr) {
	auto it = attributes.find(attr);
	if(it == attributes.end()) {
		printf("Unknown player attributes %s\n", attr.c_str());
		abort();
	} else {
		return it->second;
	}
}

void Player::change_attr(const std::string &attr, int val) {
	auto it = attributes.find(attr);
	if(it == attributes.end()) {
		printf("Unknown player attributes %s\n", attr.c_str());
		abort();
	} else {
		it->second += val;
		int max = attributes_max[attr];
		if(it->second > max)
			it->second = max;
	}
	fprintf(verbose,"%s: %d\n", attr.c_str(), it->second);
}

void Player::swing() {
	if(swing_state < 0.f) {
		//Play sound
		swing_state = 1.f;
		game.area()->attack(0);
		game.play_sfx("wroom");
	}
}

glm::vec3 Player::center3() const {
	return glm::vec3(center().x, position().y, center().y);
}


void Player::update(float dt) {
	if(swing_state >= 0.f) {
		swing_state += dt*2.f;
		if(swing_state > 2.f)
			swing_state = -1.f;

		chainsaw.set_rotation(glm::vec3(0, 1.f, 0), current_rotation-90.f);
		chainsaw.absolute_rotate(chainsaw.local_z(), M_PI_2);
		chainsaw.set_position(center3()+glm::vec3(0,1.0f, 0.0f) + local_z()); 
		chainsaw.absolute_rotate(glm::vec3(0.0, 1.f, 0.f), M_PI_2);
		chainsaw.absolute_rotate(glm::vec3(0.0, 1.f, 0.f), -(swing_state-1.f)*M_PI);
	} else {
		chainsaw.set_rotation(glm::vec3(0, 1.f, 0), current_rotation-90.f);
		chainsaw.relative_rotate(glm::vec3(1.0, 0.f, 0.f), M_PI_2);
		chainsaw.set_position(position()+glm::vec3(0,1.5f, 0) + local_x()*0.25f);
	}
	Object2D::update(dt);
}

void Player::render() {
	shaders[SHADER_NORMAL]->bind();
	chainsaw.render();
	Object2D::render();
}
