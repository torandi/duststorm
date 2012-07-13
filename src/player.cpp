#include "player.hpp"
#include "yaml-helper.hpp"
#include "game.hpp"

Player::Player(const YAML::Node &node, Game &game_) : game(game_) {
	light_color = node["light"].as<glm::vec3>(glm::vec3(0.8f));
	light_offset = node["light_offset"].as<glm::vec3>(glm::vec3(1.0));
	speed = node["speed"].as<float>(1.f);

	vfx = VFX::get_vfx(node["vfx"].as<std::string>("player"));
	vfx_state = vfx->create_state();
}

void Player::move_to(const glm::vec2 &pos) {
	set_position(glm::vec3(pos.x, game.area()->height_at(pos), pos.y));
	target = pos;
	current_position = pos;
}

void Player::render() const {
	vfx->render(matrix());
}

void Player::update(float dt) {
	vfx_state = vfx->update(dt, vfx_state);
	if(target != current_position) {
		glm::vec2 diff = target - current_position;
		if(diff.length() < speed * dt) {
			current_position = target;
		} else {
			current_position += glm::normalize(diff) * speed * dt;
		}
		update_position();
	}
}

void Player::update_position() {
	set_position(glm::vec3(current_position.x, game.area()->height_at(current_position), current_position.y));
}
