#ifndef CAMERA_H
#define CAMERA_H
	#include "movable_object.hpp"
	#include <glm/glm.hpp>

	class Camera : public MovableObject {
		float fov_;
		const float aspect_, near_, far_;

		glm::mat4 projection_matrix_;


	public:
		Camera(float fov, float aspect, float near, float far);
		virtual ~Camera() {};

		const glm::vec3 look_at() const;
		void look_at(glm::vec3 position);

		const glm::mat4 view_matrix() const;
		const glm::mat4 projection_matrix() const;

		const float fov() const;
		const float aspect() const;
		const float near() const;
		const float far() const;

		void set_fov(float fov);
	};

#endif
