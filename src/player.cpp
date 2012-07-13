#include "player.hpp"

Player::Player(const YAML::Node &node, Game &game_) : Object2D(node, game_) {
	light_color = node["light"].as<glm::vec3>(glm::vec3(0.8f));
	light_offset = node["light_offset"].as<glm::vec3>(glm::vec3(1.0)) + glm::vec3(center_offset.x, 0.f, center_offset.y);
}
