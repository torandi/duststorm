#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "hitting_particles.hpp"
#include "globals.hpp"

HittingParticles::HittingParticles(const int max_num_particles, TextureArray* texture, int max_num_enemies, bool _auto_spawn, const std::string &kernel) : ParticleSystem(max_num_particles, texture, _auto_spawn, kernel),
	max_num_enemies_(max_num_enemies)
{

	enemy_data_t * initial_enemies = new enemy_data_t[max_num_enemies];

	enemies_ = opencl->create_buffer(CL_MEM_READ_ONLY, sizeof(enemy_data_t) * max_num_enemies);
	cl_int err = opencl->queue().enqueueWriteBuffer(enemies_, CL_TRUE, 0, sizeof(enemy_data_t) * max_num_enemies, initial_enemies, NULL,NULL);

	err = run_kernel_.setArg(6, enemies_);
	CL::check_error(err, "[ParticleSystem] create hitting particles: Set arg 6");

	delete[] initial_enemies;
}

HittingParticles::~HittingParticles() { }

void HittingParticles::update(float dt, std::list<Enemy*> &enemies, Game * game) {
	cl_int err;

	enemy_list_.clear();
	for(Enemy * e : enemies) {
		enemy_data_t d = { e->position(), e->radius };
		enemy_list_.push_back(d);
		enemy_back_ref_.push_back(e);
	}

	if(enemy_list_.size() > 0) {
		err = opencl->queue().enqueueWriteBuffer(enemies_, CL_TRUE, 0, sizeof(enemy_data_t) * enemy_list_.size(), &(enemy_list_[0]), NULL,NULL);
		CL::check_error(err, "[ParticleSystem] write enemies");
	}
	err = run_kernel_.setArg(7, (unsigned int) enemies.size());
	CL::check_error(err, "[ParticleSystem] update hitting: set arg 7");

	ParticleSystem::update(dt);

	particle_t * particles = (particle_t* ) opencl->queue().enqueueMapBuffer(particles_, CL_TRUE, CL_MAP_READ, 0, sizeof(particle_t)*max_num_particles_, NULL, NULL, &err);

	CL::check_error(err, "[ParticleSystem] map buffer");

	opencl->queue().finish();

	for(int i=0; i < max_num_particles_; ++i ) {
		if(particles[i].extra1 != 0) {
			printf("HIT! %d\n", particles[i].extra1);
			Enemy * e = enemy_back_ref_[particles[i].extra1];
			//e->hp -= particles[i].extra3;
			e->hp = -1.f;
		}
	}

	
	opencl->queue().enqueueUnmapMemObject(particles_, particles, NULL, NULL);
}
