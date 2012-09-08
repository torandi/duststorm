#include <glm/glm.hpp>

#include "movable_light.hpp"
#include "globals.hpp"

MovableLight::MovableLight(Light * light) : 
		MovableObject(light->position)
	, data(light)
	, shadowmap_resolution(2048, 2048)
	, light_camera(90.f, shadowmap_resolution, 0.f, 1.f)
	, shadowmap_fbo(nullptr)
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity)
	, type(MovableLight::DIRECTIONAL_LIGHT)
	{ 
		shadowmap = Texture2D::from_filename(PATH_BASE "/textures/white.png");
		update();
	}

MovableLight::MovableLight() :
	  data(new Light())
	, shadowmap_resolution(2048, 2048)
	, light_camera(90.f, shadowmap_resolution, 0.f, 1.f)
	, shadowmap_fbo(nullptr)
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) {}

MovableLight::MovableLight(const MovableLight &ml) : MovableObject(ml.position())
	, data(ml.data)
	, shadowmap_resolution(ml.shadowmap_resolution)
	, light_camera(90.f, shadowmap_resolution, 0.f, 1.f)
	, shadowmap_fbo(nullptr)
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) { }

MovableLight::~MovableLight() {
	if(shadowmap_fbo) delete shadowmap_fbo;
}

void MovableLight::update() {
	data->position = position_;
	data->is_directional = (type == DIRECTIONAL_LIGHT);
}

void MovableLight::activate_shadowmap_rendering() {
	shadowmap_fbo = new RenderTarget(shadowmap_resolution, GL_DEPTH_COMPONENT24, RenderTarget::DEPTH_BUFFER);
}

void MovableLight::calculateFrustrumCorners(const Camera &cam, glm::vec3 * points) const {
	//Near plane:
	float top = cam.near() * tan(cam.fov() / 2.f);
	float bottom = -top;
	float left = bottom * cam.aspect();
	float right = top * cam.aspect();

	points[0] = cam.position() + glm::vec3(left, bottom, cam.near());
	points[1] = cam.position() + glm::vec3(left, top, cam.near());
	points[2] = cam.position() + glm::vec3(right, top, cam.near());
	points[3] = cam.position() + glm::vec3(right, bottom, cam.near());

	top = cam.far() * tan(cam.fov() / 2.f);
	bottom = -top;
	left = bottom * cam.aspect();
	right = top * cam.aspect();

	points[4] = cam.position() + glm::vec3(left, bottom, cam.far());
	points[5] = cam.position() + glm::vec3(left, top, cam.far());
	points[6] = cam.position() + glm::vec3(right, top, cam.far());
	points[7] = cam.position() + glm::vec3(right, bottom, cam.far());
}

void MovableLight::render_shadow_map(const Camera &camera, std::function<void(const Camera&)> render_geometry) {
	if(shadowmap_fbo == nullptr) {
		fprintf(stderr, "The developer is a moron. Call activate_shadowmap_rendering before render_shadow_map. I will now crash.\n");
		abort();
	}

	glm::vec3 direction_unit = glm::normalize(position());

	switch(type) {
		case DIRECTIONAL_LIGHT:
			//float l = glm::dot(a, b);
			break;
		default:
			printf("Shadowmaps are only implemented for directional lights at the moment\n");
			abort();
	}
}
