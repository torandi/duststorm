#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "movable_object.hpp"

#include <glm/glm.hpp>

class Player : public MovableObject {
	public:
		Player();
		~Player();

		void render_geometry(const glm::mat4 &m=glm::mat4());
		void render(const glm::mat4 &m=glm::mat4());

		void update_position(const Path * path, float pos);

		MovableObject * canon_rotation;

		const float path_position() const;

	private:
		RenderObject * model;

		Shader * shader;

		float path_position_;
};

#endif
