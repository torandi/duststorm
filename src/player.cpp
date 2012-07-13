#include "player.hpp"

Player::Player(const YAML::Node &node, Game &game_) : Object2D(node, game_) {
	light_color = node["light"].as<glm::vec3>(glm::vec3(0.8f));
	light_offset = node["light_offset"].as<glm::vec3>(glm::vec3(1.0)) + glm::vec3(center_offset.x, 0.f, center_offset.y);

	attributes["life"] = 100;
	attributes["fuel"] = 100;
	attributes["bombs"] = 2;

	const YAML::Node &n = node["weapons"];
	for(int i=0; i<4; ++i) {
		weapon_radius[i] = n[i]["radius"].as<float>();
		weapon_damage[i] = n[i]["damage"].as<float>();
	}

	click_radius = node["click_radius"].as<float>();
	hit_detection = true;
}

int &Player::attr(const std::string attr) {
	auto it = attributes.find(attr);
	if(it == attributes.end()) {
		printf("Unknown player attributes %s\n", attr.c_str());
		abort();
	} else {
		return it->second;
	}

}
