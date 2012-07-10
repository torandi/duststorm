#include "player.hpp"
#include "yaml-helper.hpp"

Player::Player(const YAML::Node &node) {
	light.intensity = node["light"].as<glm::vec3>(glm::vec3(0.8f));
	add_position_callback(&light, glm::vec3(0.f, node["light_height"].as<float>(0.1f), 0.f));
}
