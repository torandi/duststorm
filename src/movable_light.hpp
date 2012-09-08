#ifndef MOVABLE_LIGHT_H
#define MOVABLE_LIGHT_H

#include <functional>

#include <glm/glm.hpp>

#include "movable_object.hpp"
#include "light.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"
#include "camera.hpp"

class MovableLight : public MovableObject {
	private:
		Light * data;
		glm::ivec2 shadowmap_resolution;
		Camera light_camera;
		RenderTarget * shadowmap_fbo;

		void calculateFrustrumCorners(const Camera &cam, glm::vec3 * points) const;
	public:

		enum light_type_t {
			DIRECTIONAL_LIGHT, //position is direction instead
			POINT_LIGHT,
			SPOT_LIGHT 
		};

		MovableLight(Light * light);
		MovableLight();
		MovableLight(const MovableLight &ml);
		virtual ~MovableLight();

		void update(); //Must be called to update position and type in light

		void activate_shadowmap_rendering();

		float &constant_attenuation;
		float &linear_attenuation;
		float &quadratic_attenuation;
		glm::vec3 &intensity;
		light_type_t type;

		Texture2D * shadowmap;

		void render_shadow_map(const Camera &camera, std::function<void(const Camera &)> render_geometry);
};

#endif
