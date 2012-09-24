#ifndef HITTING_PARTICLES_HPP
#define HITTING_PARTICLES_HPP

#include "particle_system.hpp"
#include "enemy.hpp"

class HittingParticles : public ParticleSystem {
	public:
		HittingParticles(const int max_num_particles, TextureArray* texture, bool _auto_spawn = true, const std::string &kernel = "hitting_particles.cl");
		virtual ~HittingParticles();

		virtual void update(float dt, const std::list<Enemy*> &enemies);
	private:
		cl::Buffer enemies_;

		struct enemy_data_t {
			glm::vec4 position();
			float radius;
		};
};

#endif
