#ifndef CAMERA_H
#define CAMERA_H
	#include "movable_object.h"
	#include <glm/glm.hpp>

	class Camera : public MovableObject {
	public:
		Camera() : MovableObject() {};
		Camera(glm::vec3 position) : MovableObject(position) {};
		virtual ~Camera() {};

		const glm::vec3 look_at() const;
		const glm::vec3 up() const;
	};

#endif
