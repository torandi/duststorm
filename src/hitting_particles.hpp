#ifndef HITTING_PARTICLES_HPP
#define HITTING_PARTICLES_HPP

#include "particle_system.hpp"
#include "enemy.hpp"

class HittingParticles : public ParticleSystem {
	public:
		HittingParticles(const int max_num_particles, TextureArray* texture, int max_num_enemies, bool _auto_spawn = true, const std::string &kernel = "hitting_particles.cl");
		virtual ~HittingParticles();

		virtual void update(float dt, const std::list<Enemy*> &enemies);
	private:
		cl::Buffer enemies_;
		int max_num_enemies_;

		struct enemy_data_t {
			__ALIGNED__(glm::vec3 position, 16);
			__ALIGNED__(float radius, 16);
		};

		std::vector<enemy_data_t> enemy_list_;
};

#endif
