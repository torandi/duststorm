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

//Muste be same as in uniforms.glsl
#define MAX_NUM_LIGHTS 4

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
      NUM_GLOBAL_UNIFORMS
   };

	enum local_uniforms_t {

		UNIFORM_TEXTURE1=0,
		UNIFORM_TEXTURE2,

		NUM_LOCAL_UNIFORMS
  };

	enum attribute_t {
		ATTR_POSITION=0,
		ATTR_TEXCOORD,
		ATTR_NORMAL,
		ATTR_TANGENT,
		ATTR_BITANGENT,
		ATTR_COLOR,

		NUM_ATTRIBUTES
	};

	struct material_t {
		float shininess;
      float padding[3];
		glm::vec4 specular;
		glm::vec4 diffuse;
		glm::vec4 ambient;
		glm::vec4 emission;
	};

   struct lights_data_t {
      int num_lights;
      float padding[3];
      glm::vec3 ambient_intensity;
      float padding_2;
      Light::shader_light_t lights[MAX_NUM_LIGHTS];
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

public:

	std::string name;

	Shader &operator= (const Shader &shader);

	void bind();
	void unbind();

   /**
    * Upload lights
    */
	static void upload_lights(const lights_data_t &lights);

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


};
#endif
