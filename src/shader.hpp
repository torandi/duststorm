#ifndef SHADER_H
#define SHADER_H

#include "light.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <set>
#include <map>

#define SHADER_PATH "shaders/"
#define VERT_SHADER_EXTENTION ".vert"
#define FRAG_SHADER_EXTENTION ".frag"
#define GEOM_SHADER_EXTENTION ".geom"

class Shader {
	static GLuint load_shader(GLenum eShaderType, const std::string &strFilename);
	static GLuint create_program(const std::string &shader_name, const std::vector<GLuint> &shaderList);
	
	static void load_file(const std::string &filename, std::stringstream &shaderData, std::string included_from);
	static std::string parse_shader(const std::string &filename, std::set<std::string> included_files=std::set<std::string>(), std::string included_from="");

public:

	std::string name;

	glm::vec4 ambient_intensity;

	void upload_light(const Light &light) const;

	struct material_t {
		unsigned int use_texture;
		unsigned int use_normal_map;
		float shininess;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 ambient;
		glm::vec4 emission;
	};

	GLuint program;

	GLint Matrices;
	GLint camera_pos;
	GLint LightsData;
	GLint Material;
	GLint Camera;
	GLint texture1;
	GLint texture2;
	GLint texture_array1;
	GLint texture_array2;

	std::map<std::string, GLint> uniform; //For shader specific uniforms

	static Shader create_shader(std::string base_name);
};
#endif
