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

		const float path_position() const;

		glm::vec3 direction() const;

		glm::vec3 canon_offset;

		glm::mat4 aim_matrix() const;

		void set_canon_yaw(float angle);
		void set_canon_pitch(float angle);
	private:
		RenderObject * cart, *holder, *gun;
		MovableObject canon_pitch, canon_yaw;


		Shader * shader;

		float path_position_;
};

#endif
