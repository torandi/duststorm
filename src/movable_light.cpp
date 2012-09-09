#include <glm/glm.hpp>

#include <cfloat>
#include <glm/gtx/string_cast.hpp>

#include "movable_light.hpp"
#include "globals.hpp"

MovableLight::MovableLight(Light * light) : 
		MovableObject(light->position)
	, data(light)
	, shadow_map(glm::ivec2(2048, 2048))
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity)
	, type(MovableLight::DIRECTIONAL_LIGHT)
	{ 
		update();
	}

MovableLight::MovableLight() :
	  data(new Light())
	, shadow_map(glm::ivec2(2048, 2048))
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) {}

MovableLight::MovableLight(const MovableLight &ml) : MovableObject(ml.position())
	, data(ml.data)
	, shadow_map(glm::ivec2(2048, 2048))
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) { }

MovableLight::~MovableLight() { }

void MovableLight::update() {
	data->position = position_;
	data->is_directional = (type == DIRECTIONAL_LIGHT);
}

glm::vec3 MovableLight::calculateFrustrumData(const Camera &cam, glm::vec3 * points) const {
	//Near plane:
	float y = cam.near() * tan(cam.fov() / 2.f);
	float x = y * cam.aspect();

	glm::vec3 lx, ly, lz;
	lx = glm::normalize(cam.local_x());
	ly = glm::normalize(cam.local_y());
	lz = glm::normalize(cam.local_z());

	glm::vec3 near_center = cam.position() + lz * cam.near();
	glm::vec3 far_center = cam.position() + lz * cam.far();

	points[0] = near_center + -x * lx + -y * ly;
	points[1] = near_center + -x * lx +  y * ly;
	points[2] = near_center +  x * lx +  y * ly;
	points[3] = near_center +  x * lx + -y * ly;

	y = cam.far() * tan(cam.fov() / 2.f);
	x = y * cam.aspect();

	points[4] = far_center + -x * lx + -y * ly;
	points[5] = far_center + -x * lx +  y * ly;
	points[6] = far_center +  x * lx +  y * ly;
	points[7] = far_center +  x * lx + -y * ly;

	return near_center + (cam.far() - cam.near()) * lz;
}

void MovableLight::render_shadow_map(const Camera &camera, std::function<void()> render_geometry) {
	if(shadow_map.fbo == nullptr) shadow_map.create_fbo();

	glm::vec3 lightv[3];
	lightv[0] = glm::normalize(position());

	glm::vec3 hint;
	if(abs(lightv[0].x) <= abs(lightv[0].y)) {
		if(abs(lightv[0].x) <= abs(lightv[0].z)) {
			hint = glm::vec3(1.f, 0.f, 0.f);
		} else {
			hint = glm::vec3(0.f, 0.f, 1.f);
		}
	} else {
		if(abs(lightv[0].y) <= abs(lightv[0].z)) {
			hint = glm::vec3(0.f, 1.f, 0.f);
		} else {
			hint = glm::vec3(0.f, 0.f, 1.f);
		}
	}

	lightv[1] = glm::cross(lightv[0], hint);
	lightv[2] = glm::cross(lightv[0], lightv[1]);

	glm::vec3 frustrum_corners[8];

	glm::vec3 frustrum_center = calculateFrustrumData(camera, frustrum_corners);

	switch(type) {
		case DIRECTIONAL_LIGHT:
			{
				float min_v[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
				float max_v[3] = { FLT_MIN, FLT_MIN, FLT_MIN };
				for(glm::vec3 &corner : frustrum_corners) {
					for(int i=0; i < 3; ++i) {
						float l = glm::dot(corner, lightv[i]);
						if(l < min_v[i]) min_v[i] = l;
						if(l > max_v[i]) max_v[i] = l;
					}
				}



				break;
			}
		default:
			printf("Shadowmaps are only implemented for directional lights at the moment\n");
			abort();
	}
}

MovableLight::shadow_map_t::shadow_map_t(glm::ivec2 size) : resolution(size), fbo(nullptr), inverse_matrix(1.f){
	texture = Texture2D::from_filename(PATH_BASE "/textures/white.png");
}

MovableLight::shadow_map_t::~shadow_map_t() {
	if(fbo) delete fbo;
}

void MovableLight::shadow_map_t::create_fbo() {
	fbo = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER);
	texture = fbo;
}
