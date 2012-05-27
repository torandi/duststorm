#ifndef CAMERA_H
#define CAMERA_H

#include "movable_object.hpp"
#include <glm/glm.hpp>

class Camera : private MovableObject {
public:
	Camera(float fov, float aspect, float near, float far);
	virtual ~Camera() {};

	const glm::vec3 look_at() const;
	void look_at(const glm::vec3 &look_t);

	const glm::mat4 view_matrix() const;
	const glm::mat4 projection_matrix() const;

	const float fov() const;
	const float aspect() const;
	const float near() const;
	const float far() const;
	const float roll() const;

	void set_fov(float fov);

	virtual void roll(const float angle);
	void set_roll(const float angle);

	virtual void relative_move(const glm::vec3 &move);

	virtual void set_position(const glm::vec3 &pos);

	virtual const glm::vec3 &position() const;

private:
	void recalculate();

	float fov_, roll_;
	const float aspect_, near_, far_;

	glm::vec3 look_at_;
	glm::mat4 projection_matrix_;
};

#endif
