#ifndef SHADER_H
#define SHADER_H

#include "camera.hpp"
#include "light.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <set>

//Must be same as in uniforms.glsl
#define MAX_NUM_LIGHTS 4

class LightsData;

class Shader {
public:

	static Shader * create_shader(std::string base_name);

	//Must be called before first call to create_shader
	static void initialize();

	enum global_uniforms_t {
		UNIFORM_PROJECTION_VIEW_MATRICES=0,
		UNIFORM_MODEL_MATRICES,

		UNIFORM_CAMERA,

		UNIFORM_MATERIAL,

		UNIFORM_LIGHTS,
		UNIFORM_STATE,
		NUM_GLOBAL_UNIFORMS
	};

	enum local_uniforms_t {
		UNIFORM_TEXTURE0=0,
		UNIFORM_TEXTURE1,
		UNIFORM_TEXTURE2,
		UNIFORM_TEXTURE3,
		UNIFORM_TEXTURE4,
		UNIFORM_TEXTURE5,
		UNIFORM_TEXTURE6,
		UNIFORM_TEXTURE7,
		UNIFORM_TEXTURE_ARRAY0,
		UNIFORM_TEXTURE_ARRAY1,
		UNIFORM_TEXTURE_ARRAY2,
		UNIFORM_TEXTURE_ARRAY3,
		UNIFORM_TEXTURE_CUBE0,
		UNIFORM_TEXTURE_CUBE1,
		UNIFORM_TEXTURE_CUBE2,
		UNIFORM_TEXTURE_CUBE3,
		NUM_LOCAL_UNIFORMS
	};

	enum {
		TEXTURE_2D_0 = GL_TEXTURE0,
		TEXTURE_2D_1,
		TEXTURE_2D_2,
		TEXTURE_2D_3,
		TEXTURE_2D_4,
		TEXTURE_2D_5,
		TEXTURE_2D_6,
		TEXTURE_2D_7,
		TEXTURE_ARRAY_0,
		TEXTURE_ARRAY_1,
		TEXTURE_ARRAY_2,
		TEXTURE_ARRAY_3,
		TEXTURE_CUBEMAP_0,
		TEXTURE_CUBEMAP_1,
		TEXTURE_CUBEMAP_2,
		TEXTURE_CUBEMAP_3,
	};

	enum attribute_t {
		ATTR_POSITION=0,
		ATTR_TEXCOORD,
		ATTR_NORMAL,
		ATTR_TANGENT,
		ATTR_BITANGENT,
		ATTR_COLOR,

		NUM_ATTR,
	};

	struct vertex {
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec4 color;
	};
	typedef struct vertex vertex_t;

	struct material_t {
		float shininess;
		glm::vec4 diffuse __attribute__ ((aligned (16)));
		glm::vec4 specular;
		glm::vec4 ambient;
		glm::vec4 emission;
	};

	struct lights_data_t {
		int num_lights;
		float padding[3];
		glm::vec3 ambient_intensity;
		float padding_2;
		Light lights[MAX_NUM_LIGHTS];
	};

private:

	Shader(const std::string &name_, GLuint program);

	static const char *global_uniform_names_[];
	static const char *local_uniform_names_[];
	static const GLsizeiptr global_uniform_buffer_sizes_[];
	static const GLenum global_uniform_usage_[];

	static GLuint load_shader(GLenum eShaderType, const std::string &strFilename);
	static GLuint create_program(const std::string &shader_name, const std::vector<GLuint> &shaderList);

	static void load_file(const std::string &filename, std::stringstream &shaderData, std::string included_from);
	static std::string parse_shader(const std::string &filename, std::set<std::string> included_files=std::set<std::string>(), std::string included_from="");

	GLint local_uniform_locations_[NUM_LOCAL_UNIFORMS];
	GLint global_uniform_block_index_[NUM_GLOBAL_UNIFORMS];
	static GLuint global_uniform_buffers_[NUM_GLOBAL_UNIFORMS];

	void init_uniforms();

	const GLuint program_;

	GLint num_attributes_;

	static Shader* current; /* current bound shader or null */

public:

	std::string name;

	Shader &operator= (const Shader &shader);

	void bind();
	static void unbind();

	const GLint num_attributes() const;

	GLint uniform_location(const char * uniform_name) const;

	/**
	 * Upload lights
	 */
	static void upload_lights(const lights_data_t &lights);
	static void upload_lights(LightsData &lights);

	/*
	 * Uploads the camera position
	 */
	static void upload_camera_position(const Camera &camera);

	/**
	 * Upload the material
	 */
	static void upload_material(const material_t &material);

	/**
	 * Upload projection and view matrices
	 */
	static void upload_projection_view_matrices(
		const glm::mat4 &projection,
		const glm::mat4 &view
		);

	/**
	 * Uploads the model and normal matrix
	 */
	static void upload_model_matrix( const glm::mat4 &model);

	/**
	 * Uploads camera position, and projection and view matrix
	 */
	static void upload_camera(const Camera &camera);

	/**
	 * Upload current state.
	 */
	static void upload_state(const glm::ivec2& size);

	/**
	 * Upload white material
	 */
	static void upload_blank_material();

	const GLint uniform(local_uniforms_t uniform) const;
};
#endif
