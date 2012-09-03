#ifndef CAMERA_H
#define CAMERA_H

#include "movable_object.hpp"
#include <glm/glm.hpp>

#include "platform.h"

class Camera : public MovableObject {
public:
	Camera(float fov, float aspect, float near, float far);
	Camera(float fov, const glm::ivec2& size, float near, float far);
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
	const glm::vec3 up() const;

	void set_aspect(float aspect);
	void set_fov(float fov);

	virtual void roll(const float angle);
	void set_roll(const float angle);

	virtual void set_position(const glm::vec3 &pos);

	virtual void absolute_rotate(const glm::vec3 &axis, const float &angle);
	virtual void absolute_move(const glm::vec3 &move);
	virtual void relative_move(const glm::vec3 &move);
	virtual void relative_rotate(const glm::vec3 &axis, const float &angle);

	virtual const glm::vec3 &position() const;

private:
	void recalculate();

	float fov_, roll_;
	float aspect_, near_, far_;

	glm::vec3 look_at_;
	glm::mat4 projection_matrix_;
};

#endif
