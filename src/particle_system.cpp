#include "particle_system.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "cl.hpp"
#include "globals.hpp"
#include "utils.hpp"


ParticleSystem::ParticleSystem(const int max_num_particles) : max_num_particles_(max_num_particles) {
   program_ = opencl->create_program("cl_programs/particles.cl");
   kernel_  = opencl->load_kernel(program_, "run_particles");

   //Empty vec4s:
   glm::vec4 * empty = new glm::vec4[max_num_particles];

   //Create VBO's
   glGenBuffers(2, gl_buffers_);
   checkForGLErrors("[ParticleSystem] Generate GL buffers");

   glBindBuffer(GL_ARRAY_BUFFER, gl_buffers_[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*max_num_particles, empty, GL_DYNAMIC_DRAW);
   checkForGLErrors("[ParticleSystem] Buffer positions");

   glBindBuffer(GL_ARRAY_BUFFER, gl_buffers_[1]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*max_num_particles, empty, GL_DYNAMIC_DRAW);
   checkForGLErrors("[ParticleSystem] Buffer colors");

   glBindBuffer(GL_ARRAY_BUFFER, 0);

   delete[] empty;

   particle_t * initial_particles = new particle_t[max_num_particles];
   for(int i=0; i<max_num_particles; ++i) {
      initial_particles[i].ttl = -1.f; //mark as dead
   }

   //Create cl buffers:
   cl_gl_buffers_.push_back(opencl->create_gl_buffer(CL_MEM_READ_WRITE, gl_buffers_[0]));
   cl_gl_buffers_.push_back(opencl->create_gl_buffer(CL_MEM_READ_WRITE, gl_buffers_[1]));

   particles_ = opencl->create_buffer(CL_MEM_READ_WRITE, sizeof(particle_t)*max_num_particles);
   config_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(config_));

   cl::Event e;

   cl_int err = opencl->queue().enqueueWriteBuffer(particles_, CL_FALSE, 0, sizeof(particle_t)*max_num_particles, initial_particles, NULL, &e);
   CL::check_error(err, "[ParticleSystem] Write particles buffer");

   update_blocking_events_.push_back(e);

   delete[] initial_particles;

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

   //Set default values in config:

   config.birth_color = glm::vec4(0.f, 0.f, 1.f, 1.f);; 
   config.death_color = glm::vec4(1.f, 0.f, 0.f, 1.f);; 

   config.motion_rand = glm::vec4(0.01f, 0.01f, 0.01f, 0.f);

   config.spawn_direction = glm::vec4(1.f, 0.f, 0.f, 0.f);
   config.direction_var = glm::vec4(0.f, 0.3f, 0.f,0.f);

   config.spawn_position = glm::vec4(0, 0, 0, 0);
   config.spawn_area = glm::vec4(0.2f, 0.2f, 0.f, 0);

   //Time to live
   config.avg_ttl = 1000;
   config.ttl_var = 50;
   //Spawn speed
   config.avg_spawn_speed = 0.2f;
   config.spawn_speed_var = 0.01f;

   //Acceleration 
   config.avg_acc = -0.01f;
   config.acc_var = 0.005f;
   //Scale
   config.avg_scale = 0.1f;
   config.scale_var = 0.02f;

   update_config();
}

ParticleSystem::~ParticleSystem() {
   glDeleteBuffers(2, gl_buffers_); 
}

void ParticleSystem::update_config() {

   cl::Event e;

   cl_int err = opencl->queue().enqueueWriteBuffer(config_, CL_FALSE, 0, sizeof(config_), &config, NULL, &e);
   CL::check_error(err, "[ParticleSystem] Write config");

   update_blocking_events_.push_back(e);
}

void ParticleSystem::update(double dt) {
   cl_int err;
   //Ensure there are no pending writes active
   err = opencl->queue().enqueueWaitForEvents(update_blocking_events_);
   update_blocking_events_.clear();

   CL::check_error(err, "[ParticleSystem] Wait for events");

   //Make sure opengl is done with our vbos
   glFinish();

   update_blocking_events_[0] = cl::Event();

   err = opencl->queue().enqueueAcquireGLObjects(&cl_gl_buffers_, NULL, &update_blocking_events_[0]);
   CL::check_error(err, "[ParticleSystem] acquire gl objects");

   err = kernel_.setArg(5, (float)dt);
   CL::check_error(err, "[ParticleSystem] set dt");

   cl::Event e, e2;

   err = opencl->queue().enqueueNDRangeKernel(kernel_, cl::NullRange, cl::NDRange(max_num_particles_), cl::NullRange, &update_blocking_events_, &e);
   CL::check_error(err, "[ParticleSystem] Execute kernel");

   update_blocking_events_.clear();
   update_blocking_events_.push_back(e);

   err = opencl->queue().enqueueReleaseGLObjects(&cl_gl_buffers_, &update_blocking_events_, &e2);
   CL::check_error(err, "[ParticleSystem] Release GL objects");

   update_blocking_events_.clear();
   render_blocking_events_.push_back(e2);
   
}

void ParticleSystem::render(double dt) {
   //Ensure there are no pending updates
   cl_int err = opencl->queue().enqueueWaitForEvents(render_blocking_events_);
   CL::check_error(err, "[ParticleSystem] wait for render blocking events to complete");
   render_blocking_events_.clear();
}

void ParticleSystem::limit_particles(float limit) {
   cl_int err;
   if(limit <= max_num_particles_) {
      err = kernel_.setArg(4, limit);
      CL::check_error(err, "[ParticleSystem] Set particle limit");
   } else {
      fprintf(stderr,"[ParticleSystem] Can set particle limit higher than initial limit\n");
      abort();
   }
}
