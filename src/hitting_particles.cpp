#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "hitting_particles.hpp"
#include "globals.hpp"

HittingParticles::HittingParticles(const int max_num_particles, TextureArray* texture, bool _auto_spawn, const std::string &kernel) : ParticleSystem(max_num_particles, texture, _auto_spawn, kernel)
{
	enemies_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(config));
}

HittingParticles::~HittingParticles() { }

void HittingParticles::update(float dt, const std::list<Enemy*> &enemies) {
	std::vector<Enemy*> v(enemies.begin(), enemies.end());

	ParticleSystem::update(dt);
	cl_int err;

	particle_t * particles = (particle_t* ) opencl->queue().enqueueMapBuffer(particles_, CL_TRUE, CL_MAP_READ, 0, sizeof(particle_t)*max_num_particles_, NULL, NULL, &err);

	CL::check_error(err, "[ParticleSystem] map buffer");

	opencl->queue().finish();

	for(int i=0; i < max_num_particles_; ++i ) {
	}

	
	opencl->queue().enqueueUnmapMemObject(particles_, particles, NULL, NULL);
}
