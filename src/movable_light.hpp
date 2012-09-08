#ifndef MOVABLE_LIGHT_H
#define MOVABLE_LIGHT_H

#include <functional>

#include "movable_object.hpp"
#include "light.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"

class MovableLight : public MovableObject {
	private:
		Light * data;
		Texture2D * shadowmap;
	public:

		enum light_type_t {
			DIRECTIONAL_LIGHT, //position is direction instead
			POINT_LIGHT,
			SPOT_LIGHT 
		};

		MovableLight(Light * light);
		MovableLight();
		MovableLight(const MovableLight &ml);

		void update(); //Must be called to update position and type in light

		float &constant_attenuation;
		float &linear_attenuation;
		float &quadratic_attenuation;
		glm::vec3 &intensity;
		light_type_t type;

		void render_shadow_map(const Camera &camera, std::function<void (*)(const Camera &)> render_geometry);
};

#endif
