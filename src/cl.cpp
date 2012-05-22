#include "cl.hpp"

#include <CL/cl.hpp>
#include <vector>

#include <fstream>

CL::CL() {
   cl_int err;

   std::vector<cl::Platform> platforms;
   if(cl::Platform::get(&platforms) == CL_INVALID_VALUE) {
      fprintf(stderr, "[OpenCL] No platforms available\n");
      abort();
   }

   platform_ = platforms[0]; //Just select the first platform

   cl_context_properties properties[] = { 
            CL_CONTEXT_PLATFORM, 
            (cl_context_properties)(platform_)(), 
            0 
        };

   std::string name, version;

   platform_.getInfo(CL_PLATFORM_NAME, &name);
   platform_.getInfo(CL_PLATFORM_VERSION, &version);

   printf("[OpenCL] Platform: %s %s %ld\n", name.c_str(), version.c_str(),(cl_context_properties) (platform_)() );

   context_ = cl::Context(CL_DEVICE_TYPE_GPU, properties, &CL::cl_error_callback, NULL, &err);

   if(err != CL_SUCCESS) {
      fprintf(stderr, "[OpenCL] Failed to create context: %s \n", errorString(err), err);
      abort();
   }

   devices_ = context_.getInfo<CL_CONTEXT_DEVICES>();

   queue_ = cl::CommandQueue(context_, devices_[0], 0, &err);

   if(err != CL_SUCCESS) {
      fprintf(stderr, "[OpenCL] Failed to create a command queue: %s\n", errorString(err));
      abort();
   }
}

cl::Program CL::create_program(const char * source_file) const{
   std::ifstream file(source_file);
   if(file.fail()) {
      fprintf(stderr, "[OpenCL] Failed to open program %s\n", source_file);
      abort();
   }

   printf("Building CL program %s\n", source_file);

   std::string src = "";
   char buffer[2048];
   while(!file.eof()) {
      file.getline(buffer, 2048);
      src += std::string(buffer);
   }
   file.close();

   cl_int err;
   cl::Program::Sources source(1, std::make_pair(src.c_str(), src.size()));

   cl::Program program = cl::Program(context_, source, &err);

   if(err != CL_SUCCESS) {
      fprintf(stderr, "[OpenCL] Program creation error: %s\n", errorString(err));
   }

   err = program.build(devices_);

   if(err != CL_SUCCESS) {
      fprintf(stderr, "[OpenCL] Failed to build program: %s\n", errorString(err));
   }

   std::string build_log;
   program.getBuildInfo(devices_[0], CL_PROGRAM_BUILD_LOG, &build_log);

   if(build_log.size() > 0) {
      printf("[OpenCL] Build log: %s\n", build_log.c_str());
   }

   return program;
}

cl::Kernel CL::load_kernel(const cl::Program &program, const char * kernel_name) const{
   cl_int err;
   cl::Kernel kernel = cl::Kernel(program, kernel_name, &err);
   if(err != CL_SUCCESS) {
      fprintf(stderr,"[OpenCL] Failed to create kernel %s: %s\n", kernel_name, errorString(err));
      abort();
   }  

   return kernel;
}

cl::Buffer CL::create_buffer(cl_mem_flags flags, size_t size) const {
   cl_int err;
   cl::Buffer buffer = cl::Buffer(context_, flags, size, NULL, &err);
   if(err != CL_SUCCESS) {
      fprintf(stderr,"[OpenCL] Failed to create buffer: %s\n", errorString(err));
      abort();
   }
   return buffer;
}

cl::CommandQueue &CL::queue() { return queue_; }
cl::Context &CL::context() { return context_; }

void CL::cl_error_callback(const char * errorinfo, const void * private_info_size, size_t cb, void * user_data) {
   fprintf(stderr, "[OpenCL] Got error callback: %s\n", errorinfo);
}

const char* CL::errorString(cl_int error) {
    static const char* errorString[] = {
        "CL_SUCCESS",
        "CL_DEVICE_NOT_FOUND",
        "CL_DEVICE_NOT_AVAILABLE",
        "CL_COMPILER_NOT_AVAILABLE",
        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        "CL_OUT_OF_RESOURCES",
        "CL_OUT_OF_HOST_MEMORY",
        "CL_PROFILING_INFO_NOT_AVAILABLE",
        "CL_MEM_COPY_OVERLAP",
        "CL_IMAGE_FORMAT_MISMATCH",
        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        "CL_BUILD_PROGRAM_FAILURE",
        "CL_MAP_FAILURE",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "CL_INVALID_VALUE",
        "CL_INVALID_DEVICE_TYPE",
        "CL_INVALID_PLATFORM",
        "CL_INVALID_DEVICE",
        "CL_INVALID_CONTEXT",
        "CL_INVALID_QUEUE_PROPERTIES",
        "CL_INVALID_COMMAND_QUEUE",
        "CL_INVALID_HOST_PTR",
        "CL_INVALID_MEM_OBJECT",
        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        "CL_INVALID_IMAGE_SIZE",
        "CL_INVALID_SAMPLER",
        "CL_INVALID_BINARY",
        "CL_INVALID_BUILD_OPTIONS",
        "CL_INVALID_PROGRAM",
        "CL_INVALID_PROGRAM_EXECUTABLE",
        "CL_INVALID_KERNEL_NAME",
        "CL_INVALID_KERNEL_DEFINITION",
        "CL_INVALID_KERNEL",
        "CL_INVALID_ARG_INDEX",
        "CL_INVALID_ARG_VALUE",
        "CL_INVALID_ARG_SIZE",
        "CL_INVALID_KERNEL_ARGS",
        "CL_INVALID_WORK_DIMENSION",
        "CL_INVALID_WORK_GROUP_SIZE",
        "CL_INVALID_WORK_ITEM_SIZE",
        "CL_INVALID_GLOBAL_OFFSET",
        "CL_INVALID_EVENT_WAIT_LIST",
        "CL_INVALID_EVENT",
        "CL_INVALID_OPERATION",
        "CL_INVALID_GL_OBJECT",
        "CL_INVALID_BUFFER_SIZE",
        "CL_INVALID_MIP_LEVEL",
        "CL_INVALID_GLOBAL_WORK_SIZE",
    };

    const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

    const int index = -error;

    return (index >= 0 && index < errorCount) ? errorString[index] : "";

}