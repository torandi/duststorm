#include <glm/glm.hpp>

#include <cfloat>
#include <glm/gtc/matrix_transform.hpp>

#include "movable_light.hpp"
#include "globals.hpp"

MovableLight::MovableLight(Light * light) : 
		MovableObject(light->position)
	, data(light)
	, shadow_map(glm::ivec2(2*2048, 2*2048))
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
	, shadow_map(glm::ivec2(2*2048, 2*2048))
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) {}

MovableLight::MovableLight(const MovableLight &ml) : MovableObject(ml.position())
	, data(ml.data)
	, shadow_map(glm::ivec2(2*2048, 2*2048))
	, constant_attenuation(data->constant_attenuation)
	, linear_attenuation(data->linear_attenuation)
	, quadratic_attenuation(data->quadratic_attenuation)
	, intensity(data->intensity) { }

MovableLight::~MovableLight() { }

void MovableLight::update() {
	data->position = position_;
	data->is_directional = (type == DIRECTIONAL_LIGHT);
	data->matrix = shadow_map.matrix;
	data->shadowmap_scale = glm::vec2(1.f / shadow_map.resolution.x, 1.f / shadow_map.resolution.y);
	if(shadow_map.fbo != NULL) {
		shadow_map.fbo->depth_bind((Shader::TextureUnit) (Shader::TEXTURE_SHADOWMAP_0 + data->shadowmap_index));
	}
}

glm::vec3 MovableLight::calculateFrustrumData(const Camera &cam, float near, float far, glm::vec3 * points) const {
	//Near plane:
	float y = near * tan(cam.fov() / 2.f);
	float x = y * cam.aspect();

	glm::vec3 lx, ly, lz;
	lx = glm::normalize(cam.local_x());
	ly = glm::normalize(cam.local_y());
	lz = glm::normalize(cam.local_z());

	glm::vec3 near_center = cam.position() + lz * near;
	glm::vec3 far_center = cam.position() + lz * far;

	points[0] = near_center + -x * lx + -y * ly;
	points[1] = near_center + -x * lx +  y * ly;
	points[2] = near_center +  x * lx +  y * ly;
	points[3] = near_center +  x * lx + -y * ly;

	y = far * tan(cam.fov() / 2.f);
	x = y * cam.aspect();

	points[4] = far_center + -x * lx + -y * ly;
	points[5] = far_center + -x * lx +  y * ly;
	points[6] = far_center +  x * lx +  y * ly;
	points[7] = far_center +  x * lx + -y * ly;

	return cam.position() + far * 0.5f  * lz;
}

void MovableLight::render_shadow_map(const Camera &camera, std::function<void()> render_geometry) {
	if(shadow_map.fbo == nullptr) shadow_map.create_fbo();

	float near, far;
	near = camera.near();
	far = 100.f;

	glm::mat4 view_matrix, projection_matrix;

	switch(type) {
		case DIRECTIONAL_LIGHT:
			{
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

				lightv[1] = glm::normalize(glm::cross(lightv[0], hint));
				lightv[2] = glm::normalize(glm::cross(lightv[1], lightv[0]));

				glm::vec3 frustrum_corners[8];
				glm::vec3 frustrum_center = calculateFrustrumData(camera, near, far, frustrum_corners);

				float min_v[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
				float max_v[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
				for(glm::vec3 &corner : frustrum_corners) {
					for(int i=0; i < 3; ++i) {
						float l = glm::dot(corner, lightv[i]);
						if(l < min_v[i]) min_v[i] = l;
						if(l > max_v[i]) max_v[i] = l;
					}
				}
				float center[3];
				for(int i=0; i < 3; ++i) {
					center[i] = glm::dot(frustrum_center, lightv[i]);
					min_v[i] -= 5.f;
					max_v[i] += 5.f;
				}

				glm::vec3 eye = frustrum_center + lightv[0] * (min_v[0] - center[0]);
				float left, right, bottom, top, far_dist;

				left = min_v[1];
				right = max_v[1];
				bottom = min_v[2];
				top = max_v[2];
				far_dist = max_v[0] - min_v[0];

				float halfw, halfh;
				halfw = (right - left)/2.f;
				halfh = (top - bottom)/2.f;

				view_matrix = glm::lookAt(eye, frustrum_center, lightv[2]);
				projection_matrix = glm::ortho(-halfw, halfw, halfh, -halfh, 0.f, far_dist);

				/*glm::vec4 test4 = projection_matrix * view_matrix * glm::vec4(frustrum_center, 1.f);
				glm::vec3 test = glm::vec3(test4.x, test4.y, test4.z) / test4.w;

				printf("%f, %f, %f\n", test.x, test.y, test.z);*/

				break;
			}
		case POINT_LIGHT:
			{
				view_matrix = glm::lookAt(position(), position() + local_z(), local_y());
				projection_matrix = glm::perspective(90.f, 1.f, 0.1f, 100.f);
				break;
			}
		default:
			printf("Shadowmaps are only implemented for directional lights at the moment\n");
			abort();
	}

	shadow_map.matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.5f))
		* glm::scale(glm::mat4(1.f),glm::vec3(0.5f))
		* projection_matrix * view_matrix;

	Shader::upload_projection_view_matrices(projection_matrix, view_matrix);

	shadow_map.fbo->bind();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	shaders[SHADER_PASSTHRU]->bind();

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	render_geometry();

	shadow_map.fbo->unbind();
}

MovableLight::shadow_map_t::shadow_map_t(glm::ivec2 size) : resolution(size), fbo(nullptr), matrix(1.f){
	texture = Texture2D::from_filename(PATH_BASE "/data/textures/white.png");
}

MovableLight::shadow_map_t::~shadow_map_t() {
	if(fbo) delete fbo;
}

void MovableLight::shadow_map_t::create_fbo() {
	fbo = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER);
	texture = fbo;
}
