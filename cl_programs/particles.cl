typedef struct particle_t {
	float3 direction;
	float ttl;
	float speed;
	float acc;
	float org_ttl; //original time to live, stored to get a percentage
} particle_t ;

typedef struct vertex_t {
	float4 position;
	float4 color;
} vertex_t;

typedef struct config_t {

	float4 birth_color; 
	float4 death_color;

	float3 motion_rand; 

	float3 spawn_direction;
	float3 direction_var;

	float3 spawn_position;
	float3 spawn_area;

	float avg_ttl;
	float ttl_var;
	float avg_spawn_speed; 
	float spawn_speed_var;

	float avg_acc;
	float acc_var;

	float avg_scale;
	float scale_var;

	int max_num_particles;

} config_t;

float random(uint *time, uint id, __constant const float * rnd, int max_num_particles) {
	int i = (int)((*time)+id) % max_num_particles;
	float r = rnd[i];
	//Change time to not get same value next call:
	*time += (uint)floor(rnd[*time%max_num_particles]*1000.0);
	return r;
}

//Set dual to true to get a number in range -m..m (otherwise 0..m)
float _random1(float m, bool dual, uint *time, uint id, __constant const float * rnd, int max_num_particles) {
	return random(time, id, rnd, max_num_particles)*m*(1+dual) - m*dual;
}

float4 _random4(float4 m, bool dual, uint *time, uint id, __constant const float * rnd, int max_num_particles) {
	return (float4) (
								 _random1(m.x, dual, time, id, rnd, max_num_particles),
								 _random1(m.y, dual, time, id, rnd, max_num_particles),
								 _random1(m.z, dual, time, id, rnd, max_num_particles),
								 _random1(m.w, dual, time, id, rnd, max_num_particles)
								 );
}

float3 _random3(float3 m, bool dual, uint *time, int id, __constant const float * rnd, int max_num_particles) {
	return (float3) (
								 _random1(m.x, dual, time, id, rnd, max_num_particles),
								 _random1(m.y, dual, time, id, rnd, max_num_particles),
								 _random1(m.z, dual, time, id, rnd, max_num_particles)
								 );
}

#define random1(m, dual) _random1((m), (dual), time, id, rnd, config->max_num_particles)
#define random3(m, dual) _random3((m), (dual), time, id, rnd, config->max_num_particles)
#define random4(m, dual) _random4((m), (dual), time, id, rnd, config->max_num_particles)


void update_particle (
											__global vertex_t * const vertex, 
											__global particle_t * const particle, 
											__constant const config_t * config, 
											__constant const float * rnd,
											float dt,
											uint * time,
											uint id
											)
{
	particle->ttl -= dt;
	if(particle->ttl > 0) {
		float life_progression = 1.0 - (particle->ttl/particle->org_ttl);
		particle->speed += particle->acc;
		if(particle->speed < 0) particle->speed = 0.0;
		vertex->position.xyz += particle->direction*particle->speed + random3(config->motion_rand, true);
		vertex->color = mix(config->birth_color, config->death_color, life_progression);
	} else {
		//Dead!
		vertex->color.w = 0.0;
	}
}

void respawn_particle (
											 __global vertex_t * const vertex, 
											 __global particle_t * const particle, 
											 __constant const config_t * config, 
											 __constant const float * rnd,
											 float dt,
											 uint * time,
											 uint id
											 )
{
	vertex->position.xyz = config->spawn_position + random3(config->spawn_area, false);
	//vertex->position.w = config->avg_scale + random1(config->scale_var, true);
	vertex->position.w = 1.f;
	vertex->color = config->birth_color;

	particle->direction = normalize(config->spawn_direction + random3(config->direction_var, true));
	particle->org_ttl = particle->ttl = config->avg_ttl + random1(config->ttl_var, true);
	particle->speed = config->avg_spawn_speed + random1(config->spawn_speed_var, true);
	particle->acc = config->avg_acc + random1(config->acc_var, true);

	//Update particle
	update_particle(vertex, particle, config, rnd, dt, time, id);
}


__kernel void run_particles (
														 __global vertex_t * vertices, 
														 __global particle_t * particles, 
														 __constant const config_t * config, 
														 __constant const float * rnd,
														 int particle_limit,
														 float dt,
														 uint time
														 )
{
	uint id = get_global_id(0);    

		//First check if particle is alive:
	if(particles[id].ttl > 0) {
		//It lives
		update_particle(&vertices[id], &particles[id], config, rnd, dt, &time, id);
	} else if(id < particle_limit) {
		//Respawn!
		respawn_particle(&vertices[id], &particles[id], config, rnd, dt, &time, id);
	}
}

