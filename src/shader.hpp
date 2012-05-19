#ifndef SHADER_H
#define SHADER_H

#include "light.hpp"
#include "camera.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <set>

#define SHADER_PATH "shaders/"
#define VERT_SHADER_EXTENTION ".vert"
#define FRAG_SHADER_EXTENTION ".frag"
#define GEOM_SHADER_EXTENTION ".geom"

class Shader {
	public: 

	static Shader * create_shader(std::string base_name);

	Shader(const std::string &name_, GLuint program_);

	enum uniforms_t {
		PROJECTION_VIEW_MATRIX=0,

		PROJECTION_MATRIX,
		VIEW_MATRIX,
		MODEL_MATRIX,
		NORMAL_MATRIX,

		CAMERA_POS,

		LIGHT_ATTENUATION,
		LIGHT_INTENSITY,
		LIGHT_POSITION,

		TEXTURE1,
		TEXTURE2,

		NUM_UNIFORMS
  };

	enum attribute_t {
		ATTR_POSITION,
		ATTR_TEXCOORD,
		ATTR_NORMAL,
		ATTR_TANGENT,
		ATTR_BITANGENT,
		ATTR_COLOR,

		NUM_ATTRIBUTES
	};

	struct material_t {
		bool use_texture;
		bool use_normal_map;
		float shininess;
		glm::vec4 specular;
		glm::vec4 diffuse;
		glm::vec4 ambient;
		glm::vec4 emission;
	};

	GLint attribute_locations[NUM_ATTRIBUTES];

	private:

	static const char *uniform_names_[];
	static const char *attribute_names[];

	static GLuint load_shader(GLenum eShaderType, const std::string &strFilename);
	static GLuint create_program(const std::string &shader_name, const std::vector<GLuint> &shaderList);
	
	static void load_file(const std::string &filename, std::stringstream &shaderData, std::string included_from);
	static std::string parse_shader(const std::string &filename, std::set<std::string> included_files=std::set<std::string>(), std::string included_from="");

	GLint uniform_locations_[NUM_UNIFORMS];

	void init_uniforms();
	void init_attributes();

public:

	std::string name;
	GLuint program;

	Shader &operator= (const Shader &shader);

	void bind();
	void unbind();

	void upload_light(const Light &light) const;
	void upload_camera(const Camera &camera) const;
	void upload_projection_view_matrices(
			const glm::mat4 &projection,
			const glm::mat4 &view
		) const;

	void upload_model_matrix( const glm::mat4 &model) const;

};
#endif
