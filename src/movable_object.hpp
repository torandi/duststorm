#ifndef MOVABLE_OBJECT_H
#define MOVABLE_OBJECT_H
	#include <glm/glm.hpp>
	#include <glm/gtc/quaternion.hpp>

	#include <vector>
	#include <utility>

	#include "bindable.hpp"

class MovableObject : public Bindable {
	protected:
		mutable glm::mat4 rot_matrix_, trans_mat_, scale_matrix_, matrix_;
		mutable bool rotation_matrix_dirty_, translation_matrix_dirty_, scale_matrix_dirty_;
		glm::vec3 position_;
		glm::fquat orientation_;
		glm::vec3 scale_;


		glm::vec3 orient_vector(const glm::vec3 &vec) const;

		void orientation_changed();
		void position_changed();
		void scale_changed();

	public:
		MovableObject();
		MovableObject(glm::vec3 position);

		virtual ~MovableObject();

		virtual const glm::mat4 rotation_matrix() const;
		virtual const glm::mat4 translation_matrix() const;
		virtual const glm::mat4 scale_matrix() const;

		virtual const glm::vec3 &position() const { return position_; };
		virtual const glm::mat4 matrix() const;

		virtual void relative_move(const glm::vec3 &move);
		virtual void relative_rotate(const glm::vec3 &axis, const float &angle);

		virtual void roll(const float angle);
		virtual void pitch(const float angle);
		virtual void yaw(const float angle);

		virtual void absolute_rotate(const glm::vec3 &axis, const float &angle);
		virtual void absolute_move(const glm::vec3 &move);

		virtual void set_position(const glm::vec3 &pos);
		virtual void set_rotation(const glm::vec3 &axis, const float angle);

		virtual const glm::vec3 &scale() const;
		virtual void set_scale(const glm::vec3 &scale);
		virtual void set_scale(const float &scale);


		virtual const glm::vec3 local_x() const;
		virtual const glm::vec3 local_y() const;
		virtual const glm::vec3 local_z() const;

		virtual void callback_position(const glm::vec3 &position);
		virtual void callback_rotation(const glm::fquat &rotation);
		virtual void callback_scale(const glm::vec3 &scale);

		/**
		 * Add callback for position changes. Offset are added before the call
		 */
		virtual void add_position_callback(Bindable * obj, const glm::vec3 offset=glm::vec3(0.f));
		/**
		 * Add callback for scale changes. Scale is multiplied with factor before the call
		 */
		virtual void add_scale_callback(Bindable * obj, const glm::vec3 factor=glm::vec3(1.f));
		/**
		 * Add callback for rotation changes
		 */
		virtual void add_rotation_callback(Bindable * obj);

	private:
		std::vector<std::pair<Bindable*, glm::vec3> > position_callbacks;
		std::vector<std::pair<Bindable*, glm::vec3> > scale_callbacks;
		std::vector<Bindable*> rotation_callbacks;
	
};

#endif
