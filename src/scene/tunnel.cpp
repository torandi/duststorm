#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glm/gtc/type_ptr.hpp>

#include "quad.hpp"
#include "scene.hpp"
#include "globals.hpp"
#include "light.hpp"
#include "render_object.hpp"
#include "particle_system.hpp"
#include "shader.hpp"
#include "timetable.hpp"
#include "skybox.hpp"

static glm::vec3 offset(0,1.5,0);

static float arc[][3] = {
	{0.000000,  3.966543, 0.00f},
	{1.050000,  3.956699, 0.05f},
	{2.189144,  3.951092, 0.10f},
	{3.407661,  3.878636, 0.15f},
	{4.586273,  3.562828, 0.20f},
	{5.609610,  2.898265, 0.25f},
	{6.377500,  1.950000, 0.30f},
	{6.814777,  0.810856, 0.35f},
	{6.878636, -0.407661, 0.40f},
	{6.562828, -1.586273, 0.45f},
	{5.898265, -2.609610, 0.50f},
	{4.950000, -3.377500, 0.55f},
	{3.810856, -3.814777, 0.60f},
	{2.592339, -3.950274, 0.65f},
	{1.413727, -3.954323, 0.70f},
	{0.000000, -3.962843, 0.75f},
	{0.000000, -1.950001, 0.80f},
	{0.000000, -0.810856, 0.85f},
	{0.000000,  0.407661, 0.90f},
	{0.000000,  1.586274, 0.95f},
	{0.000000,  3.966543, 1.00f},
};
static constexpr size_t per_segment = sizeof(arc) / (3*sizeof(float));

static float noise_x(float s, unsigned int i){
	return sin(s * 28.0 + i % (per_segment-1)) * 0.2f + sin(s * 943.0f) * 0.7f;
}

static float noise_y(float s, unsigned int i){
	return sin(s * 138.0 + i % (per_segment-1)) * 0.2f + sin(s * 24.0f) * 0.3f;
}

static float noise_z(float s, unsigned int i){
	return 0.0f;
}

class TunnelScene: public Scene {
public:
	TunnelScene(const glm::ivec2& size)
		: Scene(size)
		, camera(75.f, size.x/(float)size.y, 0.1f, 100.0f)
		, position("scene/tunnel.txt") {

		lights.ambient_intensity() = glm::vec3(1.0f);
		lights.num_lights() = 0;

		camera.set_position(glm::vec3(-25,3,-25));
		camera.look_at(glm::vec3(0,0,0));

		const size_t num_segment = 8;
		const size_t num_vertices = num_segment * per_segment;
		num_indices = (per_segment-1)*4*(num_segment-1);
		//num_indices = (per_segment-1)*4;
		Shader::vertex_t vertices[num_vertices];
		unsigned int indices[num_indices];

		/* generate vertices */
		for ( unsigned int seg = 0; seg < num_segment; seg++ ){
			for ( unsigned int c = 0; c < per_segment; c++ ){
				const unsigned int i = seg * per_segment + c;

				vertices[i].pos.x = arc[c][1]  + noise_x(seg, i);
				vertices[i].pos.y = arc[c][0]  + noise_y(seg, i);
				vertices[i].pos.z = seg * 5.0f + noise_z(seg, i);
				vertices[i].uv.x = arc[c][2];
				vertices[i].uv.y = seg * 0.25f;
			}
		}

		/* generate indicies */
		unsigned int i = 0;
		for ( unsigned int seg = 0; seg < (num_segment-1); seg++ ){
			for ( unsigned int cur = 0; cur < (per_segment-1); i += 4, cur++ ){
				const unsigned offset = seg * per_segment + cur;
				indices[i+0] = offset;
				indices[i+1] = offset + per_segment;
				indices[i+2] = offset + per_segment + 1;
				indices[i+3] = offset + 1;
			}
		}

		/* upload to gpu */
		glGenBuffers(2, vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	virtual ~TunnelScene(){
		glDeleteBuffers(2, vbo);
	}

	virtual void render_geometry(const Camera& cam){
		clear(Color::yellow);

		Shader::upload_lights(lights);
		Shader::upload_camera(cam);
		shaders[SHADER_NORMAL]->bind();

		glm::mat4 model(1.f);
		Shader::upload_model_matrix(model);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glVertexAttribPointer(Shader::ATTR_POSITION,  3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, pos));
		glVertexAttribPointer(Shader::ATTR_TEXCOORD,  2, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, uv));
		glVertexAttribPointer(Shader::ATTR_NORMAL,    3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, normal));
		glVertexAttribPointer(Shader::ATTR_TANGENT,   3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, tangent));
		glVertexAttribPointer(Shader::ATTR_BITANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, bitangent));
		glVertexAttribPointer(Shader::ATTR_COLOR,     4, GL_FLOAT, GL_FALSE, sizeof(Shader::vertex_t), (const GLvoid*)offsetof(Shader::vertex_t, color));

		material.activate();		
		glDrawElements(GL_QUADS, num_indices, GL_UNSIGNED_INT, 0);
		material.deactivate();
	}

	virtual const Camera& get_current_camera(){
		return camera;
	}

	virtual void update(float t, float dt){
		camera.set_position(position.at(t) + offset);
		camera.look_at(position.at(t+0.1)  + offset);
	}

	Camera camera;
	PointTable position;
	Material material;

	GLuint vbo[2];
	size_t num_indices;
};

REGISTER_SCENE_TYPE(TunnelScene, "Tunnelbana", "tunnel.meta");
