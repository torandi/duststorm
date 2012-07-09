#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cl.hpp"
#include "data.hpp"

#include <GL/glew.h>

#ifdef HAVE_GL_GLX_H
#include <GL/glx.h>
#endif

#include <vector>
#include <sstream>

#define PP_INCLUDE "#include"

extern FILE* verbose; /* because globals.hpp fails due to libX11 containing Time which collides with our Time class */
std::map<std::string, cl::Program> CL::cache;


CL::CL() {
	cl_int err;

	std::vector<cl::Platform> platforms;
	if(cl::Platform::get(&platforms) == CL_INVALID_VALUE) {
		fprintf(stderr, "[OpenCL] No platforms available\n");
		abort();
	}

	platform_ = platforms[0]; //Just select the first platform

	std::string name, version, extensions;

	platform_.getInfo(CL_PLATFORM_NAME, &name);
	platform_.getInfo(CL_PLATFORM_VERSION, &version);
	platform_.getInfo(CL_PLATFORM_EXTENSIONS, &extensions);

	fprintf(verbose, "[OpenCL] Platform: %s %s\n"
	        "  Extensions: %s\n", name.c_str(), version.c_str() ,extensions.c_str());

#if defined (__APPLE__) || defined(MACOSX)
	CGLContextObj kCGLContext = CGLGetCurrentContext();
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
	cl_context_properties properties[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
		0
	};
#elif defined WIN32
	cl_context_properties properties[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)(platform_)(),
		0
	};
#else
	if(glXGetCurrentContext() == NULL) {
		fprintf(stderr, "[OpenCL] glXGetCurrentContex() return NULL. Make sure to create OpenGL context before create the CL-context\n");
		abort();
	}
	cl_context_properties properties[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)(platform_)(),
		0
	};
#endif


	platform_.getDevices(CL_DEVICE_TYPE_GPU, &devices_);
	cl_bool image_support, available;
	size_t max_width, max_height;
	for(cl::Device device : devices_) {
		device.getInfo(CL_DEVICE_VENDOR, &name);
		device.getInfo(CL_DEVICE_VERSION, &version);
		device.getInfo(CL_DEVICE_EXTENSIONS, &extensions);
		device.getInfo(CL_DEVICE_AVAILABLE, &available);
		device.getInfo(CL_DEVICE_IMAGE_SUPPORT, &image_support);
		device.getInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH, &max_width);
		device.getInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, &max_height);
		fprintf(verbose, "[OpenCL] Device (%p): %s %s\n"
				"		Available: %s, Image support: %s, max size: %lux%lu\n"
				"		Extensions: %s\n", (device)(),  name.c_str(), version.c_str(),available?"YES":"NO",image_support?"YES":"NO", max_width, max_height, extensions.c_str());

	}

	cl_device_id device_id;

	static CL_API_ENTRY cl_int (CL_API_CALL
	*clGetGLContextInfoKHR)(const cl_context_properties *properties,
														cl_gl_context_info param_name,
														size_t param_value_size,
														void *param_value,
														size_t *param_value_size_ret)=NULL;
	clGetGLContextInfoKHR=(clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddress("clGetGLContextInfoKHR");

	context_ = cl::Context(devices_, properties, &CL::cl_error_callback, NULL, &err);

	if(err != CL_SUCCESS) {
		fprintf(stderr, "[OpenCL] Failed to create context: %s\n", errorString(err));
		abort();
	}

	err = clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(device_id), &device_id, NULL);
	if(err != CL_SUCCESS) {
		fprintf(stderr, "[OpenCL] Failed to get current device for context: %s\n", errorString(err));
		abort();
	}
	printf("[OpenCL] Context device id: %p\n", device_id);

	context_device_ = cl::Device(device_id);

	queue_ = cl::CommandQueue(context_, context_device_, 0, &err);

	if(err != CL_SUCCESS) {
		fprintf(stderr, "[OpenCL] Failed to create a command queue: %s\n", errorString(err));
		abort();
	}
}

void CL::load_file(const std::string &filename, std::stringstream &data, const std::string &included_from) {
	Data * file = Data::open(filename);
	if(file == nullptr) {
		if(included_from.empty())
			fprintf(stderr, "[OpenCL] Kernel preprocessor error: File %s not found\n", filename.c_str());
		else
			fprintf(stderr, "[OpenCL] Kernel preprocessor error: File %s not found (included from %s)\n", filename.c_str(), included_from.c_str());
		abort();
	}
	data << file;
	delete file;
	fprintf(verbose, "[OpenCL] Loaded %s\n", filename.c_str());
}

std::string CL::parse_file(
		const std::string &filename,
		std::set<std::string> included_files,
		const std::string &included_from
	) {

	std::pair<std::set<std::string>::iterator, bool> ret = included_files.insert(filename);
	if(ret.second == false) {
		fprintf(stderr, "[OpenCL] Kernel preprocessor error: Found include loop when including %s from %s\n", filename.c_str(), included_from.c_str());
		abort();
	}

	std::stringstream raw_content, parsed_content;

	load_file(filename, raw_content, included_from);

	int linenr = 0;
	char buffer[2048];
	while(!raw_content.eof()) {
		++linenr;
		raw_content.getline(buffer, 2048);
		std::string line(buffer);
		if(line.find(PP_INCLUDE) == 0) {
			line = line.substr(line.find_first_not_of(" ", strlen(PP_INCLUDE)));

			size_t first_quote = line.find_first_of('"');
			if(first_quote != std::string::npos) {
				size_t end_quote = line.find_last_of('"');
				if(end_quote == std::string::npos || end_quote == first_quote) {
					fprintf(stderr, "%s\n[OpenCL] Kernel preprocessor error in %s:%d: Missing closing quote for #include command\n", buffer, filename.c_str(),  linenr);
					abort();
				}
				//Trim quotes
				line = line.substr(first_quote+1, (end_quote - first_quote)-1);
			}

			//Include the file:
			char loc[256];
			sprintf(loc, "%s:%d", filename.c_str(), linenr);
			parsed_content << parse_file(PATH_BASE"/cl_programs/" + line, included_files, std::string(loc));
		} else {
			parsed_content << line << std::endl;
		}
	}
	return parsed_content.str();
}

cl::Program CL::create_program(const std::string &source_file) const{
	auto it = cache.find(source_file);
	if(it != cache.end()) {
		return it->second;
	}

	fprintf(verbose, "Building CL program %s\n", source_file.c_str());

	std::string src = parse_file(PATH_BASE"/cl_programs/" + source_file, std::set<std::string>(), "");

	cl_int err;
	cl::Program::Sources source(1, std::make_pair(src.c_str(), src.size()));

	cl::Program program = cl::Program(context_, source, &err);

	if(err != CL_SUCCESS) {
		fprintf(stderr, "[OpenCL] Program creation error: %s\n", errorString(err));
	}

	err = program.build(devices_);


	std::string build_log;
	program.getBuildInfo(devices_[0], CL_PROGRAM_BUILD_LOG, &build_log);

	if(build_log.size() > 1) { /* 1+ because nvidia likes to put a single LF in the log */
		fprintf(stderr, "[OpenCL] Build log: %s\n", build_log.c_str());
	}

	if(err != CL_SUCCESS) {

		fprintf(stderr, "[OpenCL] Failed to build program: %s\n", errorString(err));
		abort();
	}

	cache[source_file] = program;

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

cl::BufferGL CL::create_gl_buffer(cl_mem_flags flags, GLuint gl_buffer) const {
	cl_int err;
	cl::BufferGL buffer(context_, flags, gl_buffer, &err);
	if(err != CL_SUCCESS) {
		fprintf(stderr,"[OpenCL] Failed to create gl buffer: %s\n", errorString(err));
		abort();
	}
	return buffer;
}

cl::Image2DGL CL::create_from_gl_2d_image(cl_mem_flags flags, Texture2D * texture, GLenum texture_target, GLint miplevel) {
	cl_int err;
	cl::Image2DGL image(context_, flags, texture_target, miplevel, texture->gl_texture(), &err);
	if(err != CL_SUCCESS) {
		fprintf(stderr,"[OpenCL] Failed to create 2D image from opengl texture: %s\n", errorString(err));
		abort();
	}
	return image;
}

cl::Image3DGL CL::create_from_gl_3d_image(cl_mem_flags flags, Texture3D * texture, GLint miplevel){
	cl_int err;
	cl::Image3DGL image(context_, flags, GL_TEXTURE_3D, miplevel, texture->gl_texture(), &err);
	if(err != CL_SUCCESS) {
		fprintf(stderr,"[OpenCL] Failed to create 3D image from opengl texture: %s\n", errorString(err));
		abort();
	}
	return image;
}

cl::CommandQueue &CL::queue() { return queue_; }
cl::Context &CL::context() { return context_; }

void CL::cl_error_callback(const char * errorinfo, const void * private_info_size, size_t cb, void * user_data) {
	fprintf(stderr, "[OpenCL] Got error callback: %s\n", errorinfo);
}

void CL::check_error(const cl_int &err, const char * context) {
	if(err != CL_SUCCESS) {
		fprintf(stderr,"[OpenCL] %s: %s\n", context, errorString(err));
		abort();
	}
}

void CL::waitForEvent(const std::vector<cl::Event> &events) {
	if(events.size() == 0)
		return;
	cl_int err = WaitForEvents(events);
	CL::check_error(err, "Wait for events");
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
