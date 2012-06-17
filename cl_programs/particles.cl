#include "particles_structs.cl"
#include "particles_random.cl"

void update_particle (
											__global vertex_t * const vertex, 
											__global particle_t * const particle, 
											__constant config_t * const config, 
											__global const float * const rnd,
											float const dt,
											uint * const time,
											uint const id
											)
{
	particle->ttl -= dt;
	if(particle->ttl > 0) {
		float life_progression = 1.0 - (particle->ttl/particle->org_ttl);
		particle->speed += particle->acc;
		if(particle->speed < 0) particle->speed = 0.0;
		vertex->position.xyz += particle->direction*particle->speed + random3(config->motion_rand, true);
		vertex->position.w += particle->rotation_speed;
		vertex->color = mix(config->birth_color, config->death_color, life_progression);
		vertex->scale = mix(particle->initial_scale, particle->final_scale, life_progression);
	} else {
		//Dead!
		vertex->color.w = 0.0;
		particle->dead = 1;
	}
}

void respawn_particle (
											 __global vertex_t * const vertex, 
											 __global particle_t * const particle, 
											 __constant config_t * const config, 
											 __global const float * const rnd,
											 const float dt,
											 uint * const time,
											 uint const id
											 )
{
	vertex->position.xyz = config->spawn_position + random3(config->spawn_area.xyz, false);
	float a = random1(2*M_PI, false);
	vertex->position.x += random1(config->spawn_area.w,false) * cos(a);
	vertex->position.z += random1(config->spawn_area.w,false) * sin(a);
	vertex->position.w = 0.f;

	vertex->texture_index = (int)floor(random1((float)config->num_textures-0.1, false));

	particle->direction = normalize(config->spawn_direction + random3(config->direction_var, true));
	particle->org_ttl = particle->ttl = config->avg_ttl + random1(config->ttl_var, true);
	particle->speed = config->avg_spawn_speed + random1(config->spawn_speed_var, true);
	particle->acc = config->avg_acc + random1(config->acc_var, true);
	particle->rotation_speed = config->avg_rotation_speed + random1(config->rotation_speed_var, true);
	particle->initial_scale = config->avg_scale + random1(config->scale_var, true);
	particle->final_scale = particle->initial_scale + config->avg_scale_change + random1(config->scale_change_var, true);
	particle->dead = 0;

	//Update particle
	update_particle(vertex, particle, config, rnd, dt, time, id);

}


__kernel void run_particles (
														 __global vertex_t * vertices, 
														 __global particle_t * particles, 
														 __constant config_t * config, 
														 __global const float * rnd,
														 __global int * to_spawn, //Number of particles left to spawn, use atomic operations!
														 float dt,
														 uint time
														 )
{
	uint id = get_global_id(0);

	//First check if particle is alive:
	if(particles[id].dead == 0) {
		//It lives
		update_particle(&vertices[id], &particles[id], config, rnd, dt, &time, id);
	} else if ( to_spawn[0] > 0 && atomic_dec(&to_spawn[0]) > 0 ) {
		//Respawn!
		respawn_particle(&vertices[id], &particles[id], config, rnd, dt, &time, id);
	}
}

