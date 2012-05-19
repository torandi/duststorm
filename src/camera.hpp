#ifndef CAMERA_H
#define CAMERA_H
	#include "movable_object.hpp"
	#include <glm/glm.hpp>

	class Camera : public MovableObject {
	public:
		Camera() : MovableObject() {};
		Camera(glm::vec3 position) : MovableObject(position) {};
		virtual ~Camera() {};

		const glm::vec3 look_at() const;
		void look_at(glm::vec3 position);
	};

#endif
