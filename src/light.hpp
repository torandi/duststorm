#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "movable_object.hpp"

#define HALF_LIGHT_DISTANCE 0.5f

class Light : public MovableObject {
	public:
		enum light_type_t {
			DIRECTIONAL_LIGHT = 0, //No attenuation
			POINT_LIGHT = 1
		} light_type;

		Light(light_type_t type, glm::vec3 _intensity);
		Light(light_type_t type, glm::vec3 _intensity, glm::vec3 position);
		virtual ~Light() {};

		struct shader_light_t {
			float attenuation;
			int type; //light_type_t
			float padding[2];
			glm::vec4 intensity;
			glm::vec4 position;
		};

		void set_half_light_distance(float hld);

		const float &attenuation() const;
		const glm::vec3 intensity() const;

		void set_intensity(glm::vec3 intensity);

		const shader_light_t &shader_light();

		const light_type_t type() const;

		void set_type(light_type_t type);

		virtual void set_position(const glm::vec3 &pos);


	private:

		shader_light_t attributes_;
};
#endif
