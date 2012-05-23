#include "particle_system.hpp"

#include "cl.hpp"
#include "globals.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>

ParticleSystem::ParticleSystem(const int max_num_particles) : max_num_particles_(max_num_particles) {
   program_ = opencl->create_program("cl_programs/particles.cl");
   run_particles_ = opencl->load_kernel(program_, "run_particles");

   //Empty vec4s:
   glm::vec4 * empty = new vec4[max_num_particles];

   //Create VBO's
   glGenBuffers(2, gl_buffers_);
   checkForGLErrors("[ParticleSystem] Generate GL buffers");

   glBindBuffers(GL_ARRAY_BUFFER, gl_buffers_[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*max_num_particles, empty, GL_DYNAMIC_DRAW);

   glBindBuffers(GL_ARRAY_BUFFER, gl_buffers_[1]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*max_num_particles, empty, GL_DYNAMIC_DRAW);

   glBindBuffers(GL_ARRAY_BUFFER, 0);

   delete[] empty;

   particle_t * initial_particles = new particle_t[max_num_particles];
   for(int i=0; i<max_num_particles; ++i) {
      initial_particles[i].ttl = -1.f; //mark as dead
   }

   //Create cl buffers:
   cl_gl_buffers_[0] = opencl->create_gl_buffer(CL_MEM_READ_WRITE, gl_buffers_[0]);
   cl_gl_buffers_[1] = opencl->create_gl_buffer(CL_MEM_READ_WRITE, gl_buffers_[1]);

   particles_ = opencl->create_buffer(CL_MEM_READ_WRITE, sizeof(particle_t)*max_num_particles);
   config_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(config_));

   cl::Event e;

   cl_int err = opencl->queue().enqueueWriteBuffer(particles_, CL_FALSE, 0, sizeof(particle_t)*max_num_particles, initial_particles, NULL, &e);
   CL::check_error(err, "[ParticleSystem] Write particles buffer");

   update_blocking_events_.push_back(e);

   delete[] empty;

   err = kernel_.setArg(0, cl_gl_buffers_[0]);
   CL::check_error(err, "[ParticleSystem] Set arg 0");
   err = kernel_.setArg(1, cl_gl_buffers_[1]);
   CL::check_error(err, "[ParticleSystem] Set arg 1");
   err = kernel_.setArg(2, particles_);
   CL::check_error(err, "[ParticleSystem] Set arg 2");
   err = kernel_.setArg(3, config_);
   CL::check_error(err, "[ParticleSystem] Set arg 3");
   err = kernel_.setArg(4, max_num_particles);
   CL::check_error(err, "[ParticleSystem] Set arg 4");

}

ParticeSystem::~ParticeSystem() {
   glDeleteBuffers(gl_buffers_); 
}

void ParticleSystem::update_config() {

   cl::Event e;

   cl_int err = opencl->queue().enqueueWriteBuffer(config_, CL_FALSE, 0, sizeof(config_), &config, NULL, &e);
   CL::check_error(err, "Write config");

   update_blocking_events_.push_back(e);
}

void ParticleSystem::update(double dt) {
   //Ensure there are no pending writes active
   opencl->queue()->enqueueWaitForEvents(update_blocking_events_);
   update_blocking_events_.clear();

   //Make sure opengl is done with our vbos
   glFinish();

   update_blocking_events_[0] = cl::Event();

   opencl->queue()->enqueueAcquireGLObjects(&cl_gl_buffers_, NULL, &update_blocking_events_[0]);

   kernel_.setArg(5, dt);

   cl::Event e, e2;

   opencl->queue()->enqueueNDRangeKernel(kernel_, cl::NullRange, cl::NDRange(max_num_particles), cl::NullRange, &update_blocking_events_, &e);

   update_blocking_events_.clear();
   update_blocking_events_.push_back(e);

   opencl->queue()->enqueueReleaseGLObjects(&cl_gl_buffers_, &update_blocking_events_, &e2);

   update_blocking_events_.clear();
   render_blocking_events_.push_back(e2);
   
}

void ParticleSystem::render(double dt) {
   //Ensure there are no pending updates
   opencl->queue()->enqueueWaitForEvents(render_blocking_events_);
   render_blocking_events_.clear();
}

void ParticleSystem::limit_particles(float limit) {
   if(limit <= max_num_particles) {
      err = kernel.setArg(4, limit);
      CL::check_error(err, "[ParticleSystem] Set particle limit");
   } else {
      fprintf(stderr,"[ParticleSystem] Can set particle limit higher than initial limit\n");
      abort();
   }
}
