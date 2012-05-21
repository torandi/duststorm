#ifndef FROB_CL_H
#define FROB_CL_H

#include <CL/cl.hpp>

#include <vector>

class CL {
   public:
      CL();

      cl::Program create_program(const char * source_file) const;

      cl::Kernel load_kernel(const cl::Program &program, const char * kernel_name) const;

      cl::Buffer create_buffer(cl_mem_flags flags, size_t size) const;

      static void cl_error_callback(const char * errorinfo, const void * private_info_size, size_t cb, void * user_data);

      static const char * errorString(cl_int error);

      cl::CommandQueue &queue();
      cl::Context  &context();

   private:

      cl::Context context_;

      cl::CommandQueue queue_;

      cl::Platform platform_;

      std::vector<cl::Device> devices_; 

};

#endif
