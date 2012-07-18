#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shader.hpp"
#include "globals.hpp"
#include "light.hpp"
#include "utils.hpp"
#include "lights_data.hpp"
#include "data.hpp"

#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <cassert>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define PP_INCLUDE "#include"
#define VERT_SHADER_EXTENTION ".vert"
#define FRAG_SHADER_EXTENTION ".frag"
#define GEOM_SHADER_EXTENTION ".geom"

struct state_data {
	float time;
	float width;
	float height;
};

const char * Shader::global_uniform_names_[] = {
	"projectionViewMatrices",
	"modelMatrices",
	"Camera",
	"Material",
	"LightsData",
	"StateData",
};

const GLsizeiptr Shader::global_uniform_buffer_sizes_[] = {
	sizeof(glm::mat4)*3,
	sizeof(glm::mat4)*2,
	sizeof(glm::vec3),
	sizeof(Shader::material_t),
	sizeof(Shader::lights_data_t),
	sizeof(struct state_data),
};

const GLenum Shader::global_uniform_usage_[] = {
	GL_DYNAMIC_DRAW,
	GL_DYNAMIC_DRAW,
	GL_DYNAMIC_DRAW,
	GL_DYNAMIC_DRAW,
	GL_DYNAMIC_DRAW,
	GL_DYNAMIC_DRAW
};

GLuint Shader::global_uniform_buffers_[Shader::NUM_GLOBAL_UNIFORMS];

Shader* Shader::current = nullptr;

typedef std::map<std::string, Shader*> ShaderMap;
typedef std::pair<std::string, Shader*> ShaderPair;
static ShaderMap shadercache;

void Shader::initialize() {
	//Generate global uniforms:
	glGenBuffers(NUM_GLOBAL_UNIFORMS, global_uniform_buffers_);

	checkForGLErrors("Generate global uniform buffers");

	for( int i = 0; i < NUM_GLOBAL_UNIFORMS; ++i) {
		//Allocate memory in the buffer:A
		glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buffers_[i]);
		glBufferData(GL_UNIFORM_BUFFER, global_uniform_buffer_sizes_[i], NULL, global_uniform_usage_[i]);
		//Bind buffers to range
		glBindBufferRange(GL_UNIFORM_BUFFER, i, global_uniform_buffers_[i], 0, global_uniform_buffer_sizes_[i]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	checkForGLErrors("Bind and allocate global uniforms");

	/* Enable all attribs for Shader::vertex_x */
	for ( int i = 0; i < NUM_ATTR; ++i ) {
		glEnableVertexAttribArray(i);
	}
}

void Shader::cleanup(){
	glDeleteBuffers(NUM_GLOBAL_UNIFORMS, global_uniform_buffers_);

	/* remove all shaders */
	for ( ShaderPair p: shadercache ){
		delete p.second;
	}
}

Shader::Shader(const std::string &name_, GLuint program) :
	program_(program)
	,	name(name_) {
	glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTES, &num_attributes_);
	fprintf(verbose, "Created shader %s\n"
	        "  ID %d\n"
	        "  Active attrib: %d\n",
	        name_.c_str(), program, num_attributes_);

	glUseProgram(program_);
	init_uniforms();
	glUseProgram(0);
}


void Shader::load_file(const std::string &filename, std::stringstream &shaderData, std::string included_from) {
	Data * file = Data::open(filename);
	if(file == nullptr) {
		if(included_from.empty())
			fprintf(stderr, "Shader preprocessor error: File %s not found\n", filename.c_str());
		else
			fprintf(stderr, "Shader preprocessor error: File %s not found (included from %s)\n", filename.c_str(), included_from.c_str());
		abort();
	}
	shaderData << file;
	delete file;
	fprintf(verbose, "Loaded %s\n", filename.c_str());
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
		abort();
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
					abort();
				}
				//Trim quotes
				line = line.substr(first_quote+1, (end_quote - first_quote)-1);
			}

			//Include the file:
			char loc[256];
			sprintf(loc, "%s:%d", filename.c_str(), linenr);
			parsed_content << parse_shader(PATH_BASE"/shaders/"+line, included_files, std::string(loc));
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
		checkForGLErrors("shader");
		abort();
	}
	return shader;
}

GLuint Shader::create_program(const std::string &shader_name, const std::vector<GLuint> &shaderList) {
	GLint gl_tmp;
	GLuint program = glCreateProgram();
	checkForGLErrors("glCreateProgram");

	for(GLuint shader : shaderList) {
		glAttachShader(program, shader);
		checkForGLErrors("glAttachShader");
	}

	glLinkProgram(program);
	checkForGLErrors("glLinkProgram");

	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	glGetProgramiv(program, GL_LINK_STATUS, &gl_tmp);

	if(!gl_tmp) {
		char buffer[2048];
		glGetProgramInfoLog(program, 2048, NULL, buffer);
		fprintf(stderr, "Link error in shader %s: %s\n", shader_name.c_str(), buffer);
		abort();
	}

#ifdef VALIDATE_SHADERS
	glValidateProgram(program);

	glGetProgramiv(program, GL_VALIDATE_STATUS, &gl_tmp);

	if(!gl_tmp) {
		char buffer[2048];
		glGetProgramInfoLog(program, 2048, NULL, buffer);
		fprintf(stderr, "Validate error in shader %s: %s\n", shader_name.c_str(), buffer);
		abort();
	}

#endif

	return program;
}

Shader* Shader::create_shader(const std::string& base_name) {
	auto it = shadercache.find(base_name);
	if ( it != shadercache.end() ){
		return it->second;
	}

	fprintf(verbose, "Compiling shader %s\n", base_name.c_str());

	const std::string vs = PATH_BASE"/shaders/"+base_name+VERT_SHADER_EXTENTION;
	const std::string gs = PATH_BASE"/shaders/"+base_name+GEOM_SHADER_EXTENTION;
	const std::string fs = PATH_BASE"/shaders/"+base_name+FRAG_SHADER_EXTENTION;

	std::vector<GLuint> shader_list;

	//Load shaders:
	shader_list.push_back(load_shader(GL_VERTEX_SHADER,   file_exists(vs) ? vs : PATH_BASE"/shaders/default.vert"));
	shader_list.push_back(load_shader(GL_FRAGMENT_SHADER, file_exists(fs) ? fs : PATH_BASE"/shaders/default.frag"));
	if ( file_exists(gs) ){
		shader_list.push_back(load_shader(GL_GEOMETRY_SHADER, gs));
	}

	return new Shader(base_name, create_program(base_name, shader_list));
}

void Shader::init_uniforms() {
	//Bind global uniforms to blocks:
	for(int i = 0; i < NUM_GLOBAL_UNIFORMS; ++i) {
		global_uniform_block_index_[i] = glGetUniformBlockIndex(program_, global_uniform_names_[i]);
		if(global_uniform_block_index_[i] != -1) {
			glUniformBlockBinding(program_, global_uniform_block_index_[i], i);
		} else {
			fprintf(verbose, "Not binding global uniform %s, probably not used\n", global_uniform_names_[i]);
		}
	}

	checkForGLErrors("Bind global uniforms to buffers");
}

void Shader::bind() {
	if ( this == current ){
		return; /* do nothing */
	} else if ( current ){
		unbind();
	}

	glUseProgram(program_);
	checkForGLErrors("Bind shader");
	current = this;
}

void Shader::unbind() {
	if ( !current ){
		fprintf(stderr, "Shader nesting problem, no shader is bound.\n");
		abort();
	}

	glUseProgram(0);
	checkForGLErrors("Shader::unbind");
	current = nullptr;
}

void Shader::upload_lights(const Shader::lights_data_t &lights) {
	glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buffers_[UNIFORM_LIGHTS]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lights_data_t), &lights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	checkForGLErrors("upload lights");
}

void Shader::upload_lights(LightsData &lights) {
	upload_lights(lights.shader_data());
}

void Shader::upload_camera_position(const Camera &camera) {
	glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buffers_[UNIFORM_CAMERA]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(camera.position()));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	checkForGLErrors("upload camera position");
}

void Shader::upload_projection_view_matrices(
	const glm::mat4 &projection,
	const glm::mat4 &view
	) {
	glm::mat4 projection_view = projection * view;

	glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buffers_[UNIFORM_PROJECTION_VIEW_MATRICES]);

	static const size_t s = sizeof(glm::mat4);
	glBufferSubData(GL_UNIFORM_BUFFER, 0*s, s, glm::value_ptr(projection_view));
	glBufferSubData(GL_UNIFORM_BUFFER, 1*s, s, glm::value_ptr(projection));
	glBufferSubData(GL_UNIFORM_BUFFER, 2*s, s, glm::value_ptr(view));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	checkForGLErrors("upload projection view matrices");
}

void Shader::upload_model_matrix(const glm::mat4 &model) {
	glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buffers_[UNIFORM_MODEL_MATRICES]);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::transpose(glm::inverse(model))));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	checkForGLErrors("upload model matrices");
}

void Shader::upload_material(const Shader::material_t &material) {
	glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buffers_[UNIFORM_MATERIAL]);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(material_t), &material);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	checkForGLErrors("upload material");
}

void Shader::upload_blank_material() {
	material_t m;
	m.shininess = 0;
	m.specular = glm::vec4(0,0,0,0);
	m.diffuse = glm::vec4(1.f,1.f,1.f,1.f);
	m.ambient = glm::vec4(1.f,1.f,1.f,1.f);
	m.emission = glm::vec4(0,0,0,0);

	upload_material(m);
}

void Shader::upload_camera(const Camera &camera) {
	upload_camera_position(camera);
	upload_projection_view_matrices(camera.projection_matrix(), camera.view_matrix());
}

void Shader::upload_state(const glm::ivec2& size){
	struct state_data data = {
		global_time,
		(float)size.x,
		(float)size.y
	};

	glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buffers_[UNIFORM_STATE]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct state_data), &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	checkForGLErrors("Shader::upload_state");
}

const GLint Shader::num_attributes() const { return num_attributes_; }

GLint Shader::uniform_location(const char * uniform_name) const{
	GLint loc = glGetUniformLocation(program_, uniform_name);
	checkForGLErrors((std::string("uniform_location(")+std::string(uniform_name)+") from shader "+name).c_str());
	return loc;
}


void Shader::push_vertex_attribs() {
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	for ( int i = 0; i < NUM_ATTR; ++i ) {
		glDisableVertexAttribArray(i);
	}
}

void Shader::pop_vertex_attribs() {
	glPopClientAttrib();
}
