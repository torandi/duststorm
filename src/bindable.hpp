#ifndef BINDABLE_HPP
#define BINDABLE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/*
 * A virtual class that can be bound to a movable object to 
 * react to that object changing
 */
class Bindable {
	public:
		/*
		 * Called when the position of the parent object changed
		 */
		virtual void callback_position(const glm::vec3 &position) = 0;

		/*
		 * Called when the rotation of the parent object changed
		 */
		virtual void callback_rotation(const glm::fquat &rotation) = 0;

		/*
		 * Called when the scale of the parent object changed
		 */
		virtual void callback_scale(const glm::vec3 &scale) = 0;
};

#endif
