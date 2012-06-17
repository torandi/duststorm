#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "particle_system.hpp"
#include "globals.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "cl.hpp"
#include "globals.hpp"
#include "utils.hpp"

ParticleSystem::ParticleSystem(const int max_num_particles, TextureArray* texture)
	:
		avg_spawn_rate(max_num_particles/10.f)
	, spawn_rate_var(avg_spawn_rate/100.f)
	,	max_num_particles_(max_num_particles)
	,	texture_(texture) {

	program_ = opencl->create_program("particles.cl");
	run_kernel_  = opencl->load_kernel(program_, "run_particles");
	spawn_kernel_  = opencl->load_kernel(program_, "spawn_particles");

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
	}

	//Create cl buffers:
	cl_gl_buffers_.push_back(opencl->create_gl_buffer(CL_MEM_READ_WRITE, gl_buffer_));

	particles_ = opencl->create_buffer(CL_MEM_READ_WRITE, sizeof(particle_t)*max_num_particles);
	config_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(config));
	spawn_rate_  = opencl->create_buffer(CL_MEM_READ_WRITE, sizeof(cl_int));

	random_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(float)*max_num_particles);
	srand(time(0));

	float * rnd = new float[max_num_particles];

	fprintf(verbose, "Generating random numbers\n");
	for(int i = 0; i<max_num_particles; ++i) {
		rnd[i] = frand();
	}

	cl_int err = opencl->queue().enqueueWriteBuffer(particles_, CL_TRUE, 0, sizeof(particle_t)*max_num_particles, initial_particles, NULL, NULL);
	CL::check_error(err, "[ParticleSystem] Write particles buffer");
	err = opencl->queue().enqueueWriteBuffer(random_, CL_TRUE, 0, sizeof(float)*max_num_particles, rnd, NULL, NULL);
	CL::check_error(err, "[ParticleSystem] Write random data buffer");

	opencl->queue().flush();

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

	config.motion_rand = glm::vec4(0.001f, 0.001f, 0.001f, 0.f);

	config.spawn_direction = glm::vec4(1.f, 0.f, 0.f, 0.f);
	config.direction_var = glm::vec4(0.f, 0.3f, 0.3f,0.f);

	config.spawn_position = glm::vec4(0.f, 0.f, 0.f, 0.f);
	config.spawn_area = glm::vec4(1.0f, 1.0f, 1.f, 0);

	//Time to live
	config.avg_ttl = 2.0;
	config.ttl_var = 1.0;
	//Spawn speed
	config.avg_spawn_speed = 0.01f;
	config.spawn_speed_var = 0.005f;

	//Acceleration
	config.avg_acc = 0.00f;
	config.acc_var = 0.000f;
	//Scale
	config.avg_scale = 0.01f;
	config.scale_var = 0.005f;
	config.avg_scale_change = 0.f;
	config.scale_change_var = 0.f;
	//Rotation
	config.avg_rotation_speed = 0.f;
	config.rotation_speed_var = 0.f;

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

void ParticleSystem::update(float dt) {
	cl_int err;

	//Make sure opengl is done with our vbos
	glFinish();

	cl::Event lock_e, e, e2, e3;

	//Write number of particles to spawn this round:
	cl_int current_spawn_rate = (cl_int) round((avg_spawn_rate + 2.f*frand()*spawn_rate_var - spawn_rate_var)*dt);
	err = opencl->queue().enqueueWriteBuffer(spawn_rate_, CL_TRUE, 0, sizeof(cl_int), &current_spawn_rate, NULL, NULL);

	err = opencl->queue().enqueueAcquireGLObjects(&cl_gl_buffers_, NULL, &lock_e);
	CL::check_error(err, "[ParticleSystem] acquire gl objects");


	err = run_kernel_.setArg(4, dt);
	CL::check_error(err, "[ParticleSystem] run: set dt");
	err = run_kernel_.setArg(5, (int)(time(0)%UINT_MAX));
	CL::check_error(err, "[ParticleSystem] run: set time");

	err = spawn_kernel_.setArg(5, (int)(time(0)%UINT_MAX));
	CL::check_error(err, "[ParticleSystem] spawn: set time");

	opencl->queue().flush();

	lock_e.wait();

	std::vector<cl::Event> queue;
	queue.push_back(cl::Event());

	err = opencl->queue().enqueueNDRangeKernel(spawn_kernel_, cl::NullRange, cl::NDRange(max_num_particles_), cl::NullRange, NULL, &queue[0]);
	CL::check_error(err, "[ParticleSystem] Execute spawn_kernel");

	opencl->queue().flush();

	err = opencl->queue().enqueueNDRangeKernel(run_kernel_, cl::NullRange, cl::NDRange(max_num_particles_), cl::NullRange, &queue, &e);
	CL::check_error(err, "[ParticleSystem] Execute run_kernel");

	//render_blocking_events_.push_back(e2);

	err = opencl->queue().enqueueReleaseGLObjects(&cl_gl_buffers_, NULL, &e2);
	CL::check_error(err, "[ParticleSystem] Release GL objects");

	opencl->queue().flush();

	e.wait();
	e2.wait();

	//render_blocking_events_.push_back(e3);


	//BEGIN DEBUG
	/*
	particle_t * particles = (particle_t*) opencl->queue().enqueueMapBuffer(particles_, CL_TRUE, CL_MAP_READ, 0, sizeof(particle_t)*max_num_particles_, NULL, NULL, &err);

	opencl->queue().finish();
	for(int i=0; i < max_num_particles_; ++i ) {
		printf("Dir: (%f, %f, %f), ttl: (%f/%f) speed: (%f) scale(%f->%f) rotation speed: %f\n", particles[i].direction.x, particles[i].direction.y,particles[i].direction.z, particles[i].ttl, particles[i].org_ttl, particles[i].speed, particles[i].initial_scale, particles[i].final_scale, particles[i].rotation_speed);
	}

	opencl->queue().enqueueUnmapMemObject(particles_, particles, NULL, NULL);
	*/
/*
	cl_int  * spawn_rate = (cl_int*) opencl->queue().enqueueMapBuffer(spawn_rate_, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_int), NULL, NULL, &err);

	opencl->queue().finish();

	printf("Spawn_rate: %d\n", *spawn_rate);

	opencl->queue().enqueueUnmapMemObject(spawn_rate_, spawn_rate, NULL, NULL);

	opencl->queue().finish();*/

	//END DEBUG
}

void ParticleSystem::render() {
	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);

	Shader::upload_model_matrix(matrix());

	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_);

/*
	//DEBUG

	vertex_t * vertices = (vertex_t* )glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

	printf("---\n");
	for(int i=0;i<max_num_particles_; ++i) {
		printf("Vertex: pos:(%f, %f, %f, %f), color:(%f, %f, %f, %f) scale: %f, texture_index: %i\n", vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, vertices[i].position.w, vertices[i].color.r, vertices[i].color.g, vertices[i].color.b, vertices[i].color.a, vertices[i].scale, vertices[i].texture_index);
	}

	printf("Sizeof(vertex_t): %d\n", sizeof(vertex_t));

	glUnmapBuffer(GL_ARRAY_BUFFER);
	//END DEBUG
*/

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (GLvoid*) sizeof(glm::vec4));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (GLvoid*) (2*sizeof(glm::vec4)));
	glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(vertex_t), (GLvoid*)		(2*sizeof(glm::vec4)+sizeof(float)));

	glActiveTexture(GL_TEXTURE0);
	texture_->texture_bind();

	glDepthMask(GL_FALSE);

	glDrawArrays(GL_POINTS, 0, max_num_particles_);

	texture_->texture_unbind();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glPopAttrib();
}
