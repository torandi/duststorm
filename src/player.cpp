#include "player.hpp"
#include "yaml-helper.hpp"

Player::Player(const YAML::Node &node) {
	light.intensity = node["light"].as<glm::vec3>(glm::vec3(0.8f));
}
