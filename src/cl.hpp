#ifndef FROB_CL_H
#define FROB_CL_H

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#pragma OPENCL EXTENSION CL_APPLE_gl_sharing : enable 
#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable

#include <GL/glew.h>

#ifdef HAVE_CL_CL_H
	#include <CL/cl.hpp>
#else
	#ifdef HAVE_OPENCL_CL_H
		#include <OpenCL/cl.hpp>
	#endif
#endif

#include <glm/glm.hpp>

#include <vector>

class CL {
   public:
      CL();

      cl::Program create_program(const char * source_file) const;

      cl::Kernel load_kernel(const cl::Program &program, const char * kernel_name) const;

      cl::Buffer create_buffer(cl_mem_flags flags, size_t size) const;
      cl::BufferGL create_gl_buffer(cl_mem_flags flags, GLuint gl_buffer) const;

      static void cl_error_callback(const char * errorinfo, const void * private_info_size, size_t cb, void * user_data);

      static void check_error(const cl_int &err, const char * context);

      static const char * errorString(cl_int error);

		static void waitForEvent(const std::vector<cl::Event> &events);

      cl::CommandQueue &queue();
      cl::Context  &context();

   private:

      cl::Context context_;

      cl::CommandQueue queue_;

      cl::Platform platform_;

      std::vector<cl::Device> devices_; 

};

#endif
