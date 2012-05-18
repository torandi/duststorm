#include "shader.hpp"
#include "light.hpp"
#include "utils.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define PP_INCLUDE "#include"

const char * Shader::uniform_names_[] = {
	"projectionViewMatrix",

	"projectionMatrix",
	"viewMatrix",
	"modelMatrix",
	"normalMatrix",

	"camera_pos",

	"light_attenuation",
	"light_intensity",
	"light_position",

	"texture1",
	"texture2"
};

Shader::Shader(const std::string &name_, GLuint program_) : name(name_), program(program_) {
	init_uniforms();
}


void Shader::load_file(const std::string &filename, std::stringstream &shaderData, std::string included_from) {
	std::ifstream shaderFile(filename.c_str());
	if(shaderFile.fail()) {
		if(included_from.empty())
			fprintf(stderr, "Shader preprocessor error: File %s not found\n", filename.c_str());
		else
			fprintf(stderr, "Shader preprocessor error: File %s not found (included from %s)\n", filename.c_str(), included_from.c_str());
		exit(2);
	}
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	printf("Loaded %s\n", filename.c_str());
}

std::string Shader::parse_shader(
			const std::string &filename,
			std::set<std::string> included_files, 
			std::string included_from
		) {
	char buffer[2048];

	std::pair<std::set<std::string>::iterator, bool> ret = included_files.insert(filename);
	if(ret.second == false) {
		fprintf(stderr, "Shader preprocessor error: Found include loop when including %s from %s\n", filename.c_str(), included_from.c_str());
		exit(2);
	}

	std::stringstream raw_content;
	load_file(filename, raw_content, included_from);
	std::stringstream parsed_content;
	int linenr = 0;
	while(!raw_content.eof()) {
		++linenr;
		raw_content.getline(buffer, 2048);
		std::string line(buffer);
		//Parse preprocessor:
		if(line.find(PP_INCLUDE) == 0) {
			line = line.substr(line.find_first_not_of(" ", strlen(PP_INCLUDE)));

			size_t first_quote = line.find_first_of('"');
			if(first_quote != std::string::npos) {
				size_t end_quote = line.find_last_of('"');
				if(end_quote == std::string::npos || end_quote == first_quote) {
					fprintf(stderr, "%s\nShader preprocessor error in %s:%d: Missing closing quote for #include command\n", buffer, filename.c_str(),  linenr);
					exit(2);
				}
				//Trim quotes
				line = line.substr(first_quote+1, (end_quote - first_quote)-1);
			}

			//Include the file:
			char loc[256];
			sprintf(loc, "%s:%d", filename.c_str(), linenr);
			parsed_content << parse_shader(SHADER_PATH+line, included_files, std::string(loc));
		} else {
			parsed_content << line << std::endl;
		}
	}
	return parsed_content.str();
}

GLuint Shader::load_shader(GLenum eShaderType, const std::string &strFilename) {
	GLint gl_tmp;

	std::string source = parse_shader(strFilename);

	GLuint shader = glCreateShader(eShaderType);

	const char * source_ptr = source.c_str();

	glShaderSource(shader, 1,&source_ptr , NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &gl_tmp);

	if(!gl_tmp) {
		char buffer[2048];

		fprintf(stderr, "Shader compile error (%s). Preproccessed source: \n", strFilename.c_str());
		std::stringstream code(source);
		int linenr=0;
		while(!code.eof()) {
			code.getline(buffer, 2048);
			fprintf(stderr, "%d %s\n", ++linenr, buffer);
		}
		glGetShaderInfoLog(shader, 2048, NULL, buffer);
		fprintf(stderr, "Error in shader %s: %s\n",strFilename.c_str(),  buffer);
		exit(2);
	} 
	return shader;
}

GLuint Shader::create_program(const std::string &shader_name, const std::vector<GLuint> &shaderList) {
	GLint gl_tmp;
	GLuint program = glCreateProgram();	

	for(GLuint shader : shaderList) {
		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	
	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	glGetProgramiv(program, GL_LINK_STATUS, &gl_tmp);

	if(!gl_tmp) {
		char buffer[2048];
		glGetProgramInfoLog(program, 2048, NULL, buffer);
		fprintf(stderr, "Link error in shader %s: %s\n", shader_name.c_str(), buffer);
		exit(2);
	}
	return program;

}

Shader Shader::create_shader(std::string base_name) {
	printf("Compiling shader %s\n", base_name.c_str());

	std::vector<GLuint> shader_list;
	//Load shaders:
	shader_list.push_back(load_shader(GL_VERTEX_SHADER, SHADER_PATH+base_name+VERT_SHADER_EXTENTION));
	//Check if geometry shader exists:
	std::string geom_shader = SHADER_PATH+base_name+GEOM_SHADER_EXTENTION;
	std::ifstream file(geom_shader.c_str());
	if(file)
		shader_list.push_back(load_shader(GL_GEOMETRY_SHADER, geom_shader));
	shader_list.push_back(load_shader(GL_FRAGMENT_SHADER, SHADER_PATH+base_name+FRAG_SHADER_EXTENTION));
	
	Shader shader(base_name, create_program(base_name, shader_list));

	return shader;
}

void Shader::init_uniforms() {
	for(int i=0; i<NUM_UNIFORMS; ++i) {
		uniform_locations_[i] = glGetUniformLocation(program, uniform_names_[i]);
		checkForGLErrors((std::string("load uniform ")+uniform_names_[i]+" from shader "+name).c_str());
	}
	bind();
	glUniform1i(TEXTURE1, 0);
	glUniform1i(TEXTURE2, 1);
	unbind();
}

void Shader::bind() {
	glUseProgram(program);
}

void Shader::unbind() {
	glUseProgram(0);
}

void Shader::upload_light(const Light &light) const {
	glUniform1f(uniform_locations_[LIGHT_ATTENUATION], light.attenuation);
	glUniform4fv(uniform_locations_[LIGHT_INTENSITY], 1, glm::value_ptr(light.intensity));
	glUniform4fv(uniform_locations_[LIGHT_POSITION], 1, glm::value_ptr(light.position()));
}

void Shader::upload_camera(const Camera &camera) const {
	glUniform3fv(uniform_locations_[CAMERA_POS], 1, glm::value_ptr(camera.position()));
}

void Shader::upload_matrices(
		const glm::mat4 &model,
		const glm::mat4 &view,
		const glm::mat4 &projection
	) const {
		glUniform4fv(uniform_locations_[MODEL_MATRIX], 1, glm::value_ptr(model));	
		glUniform4fv(uniform_locations_[VIEW_MATRIX], 1, glm::value_ptr(view));	
		glUniform4fv(uniform_locations_[PROJECTION_MATRIX], 1, glm::value_ptr(projection));	
		glUniform4fv(uniform_locations_[PROJECTION_VIEW_MATRIX], 1, glm::value_ptr(view * projection));
		glUniform4fv(uniform_locations_[NORMAL_MATRIX],1 , glm::value_ptr(glm::transpose(glm::inverse(model))));
}
