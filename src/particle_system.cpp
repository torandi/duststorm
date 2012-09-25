#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "particle_system.hpp"
#include "globals.hpp"
#include "texture.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <ctime>

#include "cl.hpp"
#include "globals.hpp"
#include "utils.hpp"

ParticleSystem::ParticleSystem(const int max_num_particles, TextureArray* texture, bool _auto_spawn, const std::string & kernel)
	:
		avg_spawn_rate(max_num_particles/10.f)
	, spawn_rate_var(avg_spawn_rate/100.f)
	, auto_spawn(_auto_spawn)
	,	max_num_particles_(max_num_particles)
	,	texture_(texture) {

	program_ = opencl->create_program(kernel);
	run_kernel_  = opencl->load_kernel(program_, "run_particles");
	spawn_kernel_  = opencl->load_kernel(program_, "spawn_particles");

	fprintf(verbose,"Created particle system with %d particles\n", max_num_particles);

	//Empty vec4s:
	vertex_t * empty = new vertex_t[max_num_particles];

	for(int i=0;i<max_num_particles; ++i) {
		empty[i].position = glm::vec4(0.f);
		empty[i].color = glm::vec4(0.f);
		empty[i].scale = 0.f;
		empty[i].texture_index = 0;
	}

	//Create VBO's
	glGenBuffers(1, &gl_buffer_);
	checkForGLErrors("[ParticleSystem] Generate GL buffer");

	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t)*max_num_particles, empty, GL_DYNAMIC_DRAW);

	checkForGLErrors("[ParticleSystem] Buffer vertices");

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] empty;

	particle_t * initial_particles = new particle_t[max_num_particles];
	for(int i=0; i<max_num_particles; ++i) {
		initial_particles[i].dead = 1; //mark as dead
		initial_particles[i].extra1 = -1;
		initial_particles[i].extra2 = -1;
		initial_particles[i].extra3 = -1;
		initial_particles[i].extra4 = -1;
	}

	//Create cl buffers:
	cl_gl_buffers_.push_back(opencl->create_gl_buffer(CL_MEM_READ_WRITE , gl_buffer_));

	particles_ = opencl->create_buffer(CL_MEM_READ_WRITE, sizeof(particle_t)*max_num_particles);
	config_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(config));
	spawn_rate_  = opencl->create_buffer(CL_MEM_READ_WRITE, sizeof(cl_int));

	random_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(float)*max_num_particles);

	float * rnd = new float[max_num_particles];

	fprintf(verbose, "Generating random numbers\n");
	for(int i = 0; i<max_num_particles; ++i) {
		rnd[i] = frand();
	}

	cl::Event lock[2];

	cl_int err = opencl->queue().enqueueWriteBuffer(particles_, CL_FALSE, 0, sizeof(particle_t)*max_num_particles, initial_particles, NULL, &lock[0]);
	CL::check_error(err, "[ParticleSystem] Write particles buffer");
	err = opencl->queue().enqueueWriteBuffer(random_, CL_FALSE, 0, sizeof(float)*max_num_particles, rnd, NULL, &lock[1]);
	CL::check_error(err, "[ParticleSystem] Write random data buffer");

	opencl->queue().flush();

	lock[0].wait();
	lock[1].wait();

	delete[] initial_particles;
	delete[] rnd;

	err = run_kernel_.setArg(0, cl_gl_buffers_[0]);
	CL::check_error(err, "[ParticleSystem] run: Set arg 0");
	err = run_kernel_.setArg(1, particles_);
	CL::check_error(err, "[ParticleSystem] run: Set arg 1");
	err = run_kernel_.setArg(2, config_);
	CL::check_error(err, "[ParticleSystem] run: Set arg 2");
	err = run_kernel_.setArg(3, random_);
	CL::check_error(err, "[ParticleSystem] run: Set arg 3");

	err = spawn_kernel_.setArg(0, cl_gl_buffers_[0]);
	CL::check_error(err, "[ParticleSystem] spawn: Set arg 0");
	err = spawn_kernel_.setArg(1, particles_);
	CL::check_error(err, "[ParticleSystem] spawn: Set arg 1");
	err = spawn_kernel_.setArg(2, config_);
	CL::check_error(err, "[ParticleSystem] spawn: Set arg 2");
	err = spawn_kernel_.setArg(3, random_);
	CL::check_error(err, "[ParticleSystem] spawn: Set arg 3");
	err = spawn_kernel_.setArg(4, spawn_rate_);
	CL::check_error(err, "[ParticleSystem] spawn: Set arg 4");

	//Set default values in config:

	config.birth_color = glm::vec4(0.f, 1.f, 1.f, 1.f);;
	config.death_color = glm::vec4(1.f, 0.f, 0.f, 1.f);;

	config.motion_rand = glm::vec4(0.f, 0.f, 0.f, 0.f);

	config.avg_spawn_velocity = glm::vec4(1.f, 0.f, 0.f, 0.f);
	config.spawn_velocity_var = glm::vec4(0.f, 0.3f, 0.3f,0.f);

	config.spawn_position = glm::vec4(0.f, 0.f, 0.f, 0.f);
	config.spawn_area = glm::vec4(1.0f, 1.0f, 1.f, 0);

	config.wind_velocity = glm::vec4(0.f);
	config.gravity = glm::vec4(0, -1.f, 0, 0);

	//Time to live
	config.avg_ttl = 2.0;
	config.ttl_var = 1.0;

	//Scale
	config.avg_scale = 0.01f;
	config.scale_var = 0.005f;
	config.avg_scale_change = 0.f;
	config.scale_change_var = 0.f;

	//Rotation
	config.avg_rotation_speed = 0.f;
	config.rotation_speed_var = 0.f;

	config.avg_wind_influence = 0.1f;
	config.wind_influence_var = 0.f;
	config.avg_gravity_influence = 0.5f;
	config.gravity_influence_var = 0.f;

	config.start_texture = 0;
	config.num_textures = texture->num_textures();
	config.max_num_particles = max_num_particles;
	update_config();

}

ParticleSystem::~ParticleSystem() {
	glDeleteBuffers(1, &gl_buffer_);
}

void ParticleSystem::update_config() {
	cl_int err = opencl->queue().enqueueWriteBuffer(config_, CL_TRUE, 0, sizeof(config), &config, NULL, NULL);
	CL::check_error(err, "[ParticleSystem] Write config");
}


void ParticleSystem::callback_position(const glm::vec3 &position) {
	config.spawn_position = glm::vec4(position,1.f);
	update_config();
}

void ParticleSystem::spawn_particles(cl_int count, cl::Event * event) {
	cl_int err = opencl->queue().enqueueWriteBuffer(spawn_rate_, CL_TRUE, 0, sizeof(cl_int), &count, NULL, NULL);
	CL::check_error(err, "[ParticleSystem] spawn: Write spawn count");

	err = spawn_kernel_.setArg(5, (int)(time(0)%UINT_MAX));
	CL::check_error(err, "[ParticleSystem] spawn: set time");
	//TODO: Optimize!
	err = opencl->queue().enqueueNDRangeKernel(spawn_kernel_, cl::NullRange, cl::NDRange(max_num_particles_), cl::NullRange, NULL, event);
	CL::check_error(err, "[ParticleSystem] Execute spawn_kernel");
	//opencl->queue().finish();
}

void ParticleSystem::update(float dt) {
	cl_int err;

	//Make sure opengl is done with our vbos
	glFinish();

	std::vector<cl::Event> lock(1,cl::Event());

	err = opencl->queue().enqueueAcquireGLObjects((std::vector<cl::Memory>*) &cl_gl_buffers_, NULL, &lock[0]);
	CL::check_error(err, "[ParticleSystem] acquire gl objects");
	
	err = spawn_kernel_.setArg(5, (int)(time(0)%UINT_MAX));
	CL::check_error(err, "[ParticleSystem] spawn: set time");

	opencl->queue().flush();
	lock[0].wait(); //Wait to aquire gl objects
	
	bool restore_config = !spawn_list_.empty();
	
	// handle spawning
	while(!spawn_list_.empty()) {
		spawn_data &sd = spawn_list_.front();

		cl_int err = opencl->queue().enqueueWriteBuffer(config_, CL_TRUE, 0, sizeof(config_t), &sd.first, NULL, NULL);
		CL::check_error(err, "[ParticleSystem] Write config");
	
		spawn_particles((cl_int) sd.second, &lock[0]);

		spawn_list_.pop_front();

		opencl->queue().flush();

		lock[0].wait();
	}
	
	if(restore_config) {
		update_config();
	}
	
	if(auto_spawn) {
		//Write number of particles to spawn this round:
		cl_int current_spawn_rate = (cl_int) round((avg_spawn_rate + 2.f*frand()*spawn_rate_var - spawn_rate_var)*dt);
		spawn_particles(current_spawn_rate, &lock[0]);
		opencl->queue().flush();
		lock[0].wait();
	}
	
	
	
	err = run_kernel_.setArg(4, dt);
	CL::check_error(err, "[ParticleSystem] run: set dt");
	err = run_kernel_.setArg(5, (int)(time(0)%UINT_MAX));
	CL::check_error(err, "[ParticleSystem] run: set time");
	
	err = opencl->queue().enqueueNDRangeKernel(run_kernel_, cl::NullRange, cl::NDRange(max_num_particles_), cl::NullRange, NULL, &lock[0]);
	CL::check_error(err, "[ParticleSystem] Execute run_kernel");
	
	
	err = opencl->queue().enqueueReleaseGLObjects((std::vector<cl::Memory>*)&cl_gl_buffers_, &lock, NULL);
	CL::check_error(err, "[ParticleSystem] Release GL objects");
	
	opencl->queue().flush();
	
	opencl->queue().finish();
}

void ParticleSystem::render(const glm::mat4 * m) {

	Shader::push_vertex_attribs();

	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	if(m == nullptr) 
		Shader::upload_model_matrix(matrix());
	else
		Shader::upload_model_matrix(matrix() * (*m));

	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_);

	//DEBUG
/*
	vertex_t * vertices = (vertex_t* )glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

	printf("---\n");
	for(int i=0;i<max_num_particles_; ++i) {
		printf("Vertex: pos:(%f, %f, %f, %f), color:(%f, %f, %f, %f) scale: %f, texture_index: %i\n", vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, vertices[i].position.w, vertices[i].color.r, vertices[i].color.g, vertices[i].color.b, vertices[i].color.a, vertices[i].scale, vertices[i].texture_index);
	}

	printf("Sizeof(vertex_t): %d\n", sizeof(vertex_t));

	if(!glUnmapBuffer(GL_ARRAY_BUFFER)) {
		printf("glUnmapBuffer returned False\n");
		abort();
	}
	*/


	//END DEBUG

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, color));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, scale));
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, texture_index));
	texture_->texture_bind(Shader::TEXTURE_ARRAY_0);

	glDrawArrays(GL_POINTS, 0, max_num_particles_);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glPopAttrib();

	Shader::pop_vertex_attribs();

}

void ParticleSystem::push_config() {
	config_stack_.push_back(config);
}

void ParticleSystem::pop_config() {
	config = config_stack_.back();
	config_stack_.pop_back();
}

void ParticleSystem::spawn(int count) {
	spawn_data sd;
	sd.first = config;
	sd.second = count;
	spawn_list_.push_back(sd);
}
