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

	public:
		/**
		 * points shall be [8], filled with corners
		 * @return frustrum center
		 */
		glm::vec3 calculateFrustrumData(const Camera &cam, float near, float far, glm::vec3 * points) const;

		enum light_type_t {
			DIRECTIONAL_LIGHT, //position is direction instead
			POINT_LIGHT,
			SPOT_LIGHT 
		};

		struct shadow_map_t {
			shadow_map_t(glm::ivec2 size);
			~shadow_map_t();

			void create_fbo();

			glm::ivec2 resolution;
			RenderTarget * fbo;
			TextureBase * texture;
			glm::mat4 matrix;
		} shadow_map;

		MovableLight(Light * light);
		MovableLight();
		MovableLight(const MovableLight &ml);
		virtual ~MovableLight();

		void update(); //Must be called to update position and type in light

		float &constant_attenuation;
		float &linear_attenuation;
		float &quadratic_attenuation;
		glm::vec3 &intensity;
		light_type_t type;

		void render_shadow_map(const Camera &camera, std::function<void()> render_geometry);
};

#endif
