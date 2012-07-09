#ifndef FROB_CL_H
#define FROB_CL_H

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <GL/glew.h>

#include <CL/cl.hpp>

#include <glm/glm.hpp>
#include "texture.hpp"

#include <string>
#include <vector>
#include <map>
#include <set>

class CL {
   public:
      CL();

      cl::Program create_program(const std::string &file_name) const;

      cl::Kernel load_kernel(const cl::Program &program, const char * kernel_name) const;

      cl::Buffer create_buffer(cl_mem_flags flags, size_t size) const;
      cl::BufferGL create_gl_buffer(cl_mem_flags flags, GLuint gl_buffer) const;

			cl::Image2DGL create_from_gl_2d_image(cl_mem_flags flags, Texture2D * texture, GLenum texture_target=GL_TEXTURE_2D, GLint miplevel = 0);
			cl::Image3DGL create_from_gl_3d_image(cl_mem_flags flags, Texture3D * texture, GLint miplevel = 0);

      static void cl_error_callback(const char * errorinfo, const void * private_info_size, size_t cb, void * user_data);

      static void check_error(const cl_int &err, const char * context);

      static const char * errorString(cl_int error);

			static void waitForEvent(const std::vector<cl::Event> &events);

      cl::CommandQueue &queue();
      cl::Context  &context();

   private:

			static std::string parse_file(
					const std::string &filename,
					std::set<std::string> included_files,
					const std::string &included_from);

			static void load_file(const std::string &filename, std::stringstream &data, const std::string &included_from);

			static std::map<std::string, cl::Program> cache;

      cl::Context context_;

      cl::CommandQueue queue_;

      cl::Platform platform_;

      std::vector<cl::Device> devices_; 
			cl::Device context_device_;

};

#endif
