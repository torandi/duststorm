#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "movable_object.hpp"

#include <cstdio>

MovableObject::MovableObject() : orientation_(1.f, 0.f, 0.f,0.f), scale_(1.f) {
	rotation_matrix_dirty_ = true;
	translation_matrix_dirty_ = true;
	scale_matrix_dirty_ = true;
};

MovableObject::MovableObject(glm::vec3 position) : position_(position), orientation_(1.f, 0.f, 0.f,0.f), scale_(1.f) { 
	rotation_matrix_dirty_ = true;
	translation_matrix_dirty_ = true;
	scale_matrix_dirty_ = true;
}

MovableObject::~MovableObject() { }	

const glm::mat4 MovableObject::translation_matrix() const{
	if(translation_matrix_dirty_) {
		trans_mat_ = glm::translate(glm::mat4(1.0f), position());
		translation_matrix_dirty_ = false;
	}
	return trans_mat_;
}

const glm::mat4 MovableObject::rotation_matrix() const{
	if(rotation_matrix_dirty_) {
		rot_matrix_ = glm::mat4_cast(orientation_);
		rotation_matrix_dirty_ = false;
	}
	return rot_matrix_;
}

const glm::mat4 MovableObject::matrix() const{
	if(translation_matrix_dirty_ 
			|| rotation_matrix_dirty_ 
			|| scale_matrix_dirty_)  {
		matrix_ = translation_matrix()*rotation_matrix()*scale_matrix();
	}
	return matrix_;
}

const glm::mat4 MovableObject::scale_matrix() const {
	if(scale_matrix_dirty_) {
		scale_matrix_ = glm::scale(glm::mat4(1.f), scale_);
		scale_matrix_dirty_ = false;
	}
	return scale_matrix_;
}

void MovableObject::relative_move(const glm::vec3 &move) {
	translation_matrix_dirty_ = true;
	position_+= orient_vector(move);
	position_changed();
}

void MovableObject::absolute_rotate(const glm::vec3 &axis, const float &angle) {
	rotation_matrix_dirty_ = true;
	glm::vec3 n_axis = glm::normalize(axis) * sinf(angle/2.f);
	glm::fquat offset = glm::fquat(cosf(angle/2.f), n_axis.x, n_axis.y, n_axis.z);

	orientation_ = offset * orientation_ ;
	orientation_ = glm::normalize(orientation_);
	orientation_changed();
}

void MovableObject::absolute_move(const glm::vec3 &move) {
	translation_matrix_dirty_ = true;
	position_ += move;
	position_changed();
}

void MovableObject::relative_rotate(const glm::vec3 &axis, const float &angle) {
	rotation_matrix_dirty_ = true;
   glm::vec3 n_axis = glm::normalize(axis) * sinf(angle/2.f);
   glm::fquat offset = glm::fquat(cosf(angle/2.f), n_axis.x, n_axis.y, n_axis.z);

   orientation_ = orientation_  * offset;
   orientation_ = glm::normalize(orientation_);
	orientation_changed();
}

const glm::vec3 &MovableObject::scale() const { return scale_; }

void MovableObject::set_scale(const float &scale) {
	set_scale(glm::vec3(scale));
	scale_changed();
}

void MovableObject::set_scale(const glm::vec3 &scale) {
	scale_ = scale;
	scale_matrix_dirty_ = true;
	scale_changed();
}

void MovableObject::set_position(const glm::vec3 &pos) {
	position_ = pos;
	translation_matrix_dirty_ = true;
	position_changed();
}

void MovableObject::set_rotation(const glm::vec3 &axis, const float angle) {
	rotation_matrix_dirty_ = true;
	orientation_ = glm::rotate(glm::fquat(1.f, 0.f, 0.f, 0.f), angle, axis);
	orientation_changed();
}

void MovableObject::roll(const float angle) {
	relative_rotate(glm::vec3(0.f, 0.f, 1.f), angle);
}

void MovableObject::pitch(const float angle) {
	relative_rotate(glm::vec3(1.f, 0.f, 0.f), angle);
}

void MovableObject::yaw(const float angle) {
	relative_rotate(glm::vec3(0.f, 1.f, 0.f), angle);
}

const glm::vec3 MovableObject::local_z() const {
	return glm::vec3(rotation_matrix()*glm::vec4(0.f, 0.f, 1.f, 1.f));
}

const glm::vec3 MovableObject::local_y() const {
	return glm::vec3(rotation_matrix()*glm::vec4(0.f, 1.f, 0.f, 1.f));
}

const glm::vec3 MovableObject::local_x() const {
	return glm::vec3(rotation_matrix()*glm::vec4(1.f, 0.f, 0.f, 1.f));
}

glm::vec3 MovableObject::orient_vector(const glm::vec3 &vec) const {
   return glm::vec3(rotation_matrix()*glm::vec4(vec, 1.f));
}

void MovableObject::callback_position(const glm::vec3 &position) {
	position_ = position;
	translation_matrix_dirty_ = true;
	position_changed();
}

void MovableObject::callback_rotation(const glm::fquat &rotation) {
	orientation_ = rotation;
	rotation_matrix_dirty_ = true;
	orientation_changed();
}

void MovableObject::callback_scale(const glm::vec3 &scale) {
	scale_ = scale;
	scale_matrix_dirty_ = true;
	scale_changed();
}

void MovableObject::orientation_changed() {
	for(auto callback : rotation_callbacks) {
		callback->callback_rotation(orientation_);
	}
}

void MovableObject::position_changed() {
	for(auto callback : position_callbacks) {
		callback.first->callback_position(position_ + callback.second);
	}
}

void MovableObject::scale_changed() {
	for(auto callback : scale_callbacks) {
		callback.first->callback_scale(scale_ * callback.second);
	}
}

void MovableObject::add_position_callback(Bindable * obj, const glm::vec3 offset) {
	position_callbacks.push_back(std::pair<Bindable*, glm::vec3>(obj, offset));
}

void MovableObject::add_scale_callback(Bindable * obj, const glm::vec3 factor) {
	scale_callbacks.push_back(std::pair<Bindable*, glm::vec3>(obj, factor));
}

void MovableObject::add_rotation_callback(Bindable * obj) {
	rotation_callbacks.push_back(obj);
}
