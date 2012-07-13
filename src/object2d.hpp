#ifndef OBJECT2D_HPP
#define OBJECT2D_HPP

#include "movable_object.hpp"
#include "yaml-helper.hpp"
#include "vfx.hpp"

class Game;

class Object2D : public MovableObject {
	public:
		Object2D(const YAML::Node &node, Game &game_);
		Object2D(const std::string &vfx_name, Game &game_);
		glm::vec2 target;
		glm::vec2 current_position;
		float speed, radius;
		bool hit_detection; //true to enable
		float height;

		virtual bool hit(const Object2D * other) const;
		virtual bool hit(const glm::vec2 &pos, float radius) const;

		virtual void move_to(const glm::vec2 &pos);

		virtual void update_position();

		virtual void render() const;
		virtual void update(float dt);

		virtual void rotate(float angle);
		virtual void set_rotation(float angle);

		virtual void face(const glm::vec2 &pos);
		virtual void face(const Object2D * obj);

		glm::vec2 center() const;
		Game &game;
	protected:

		VFX * vfx;
		void * vfx_state;
		float base_rotation;
		glm::vec2 center_offset;

};

#endif
