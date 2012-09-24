#include "particles_structs.cl"
#include "particles_random.cl"

typedef struct {
	float4 position;
	float radius;
} enemy_data_t __attribute__ ((aligned (16)));

__kernel void run_particles (
														 __global vertex_t * vertices, 
														 __global particle_t * particles, 
														 __constant config_t * config, 
														 __global const float * rnd,
														 float dt,
														 uint time,
														 __global const enemy_data_t * enemies,
														 uint num_enemies
														 )
{
	uint id = get_global_id(0);
	if(particles[id].dead == 0) {
		particles[id].ttl -= dt;
		if(particles[id].ttl > 0) {
			float life_progression = 1.0 - (particles[id].ttl/particles[id].org_ttl);

			particles[id].velocity += config->gravity.xyz * particles[id].gravity_influence * dt;
			particles[id].velocity -= (particles[id].velocity - config->wind_velocity.xyz) * particles[id].wind_influence * dt;
			//particles[id].velocity += config->wind_velocity * particles[id].wind_influence * dt;

			vertices[id].position.xyz += (particles[id].velocity + random3(config->motion_rand.xyz, true)) * dt;
			vertices[id].position.w += particles[id].rotation_speed * dt;

			vertices[id].color = mix(config->birth_color, config->death_color, life_progression);
			//vertices[id].color = (float4)(particles[id].wind_influence);
			vertices[id].scale = mix(particles[id].initial_scale, particles[id].final_scale, life_progression);

			//TODO: Collision detection (ground and enemies)
			//bool collision = false;
		} else {
			//Dead!
			vertices[id].color.w = 0.0;
			vertices[id].scale = 0.0;
			particles[id].dead = 1;
		}
	}

}

__kernel void spawn_particles (
														 __global vertex_t * vertices, 
														 __global particle_t * particles, 
														 __constant config_t * config, 
														 __global const float * rnd,
														 __global int * to_spawn, //Number of particles left to spawn, use atomic operations!
														 uint time
														 )
{
	uint id = get_global_id(0);

	if (particles[id].dead == 1 && to_spawn[0] > 0 && atomic_dec(&to_spawn[0]) > 0 ) {
		vertices[id].position.xyz = config->spawn_position.xyz + random3(config->spawn_area.xyz, false);

		//Save colors to allow changing config during runtime
		particles[id].birth_color = config->birth_color;
		particles[id].death_color = config->death_color;

		float a = random1(2*M_PI, false);
		float a2 = random1(2*M_PI, false);
		float len = random1(config->spawn_area.w,false);
		vertices[id].position.x += len * cos(a);
		vertices[id].position.y += len * sin(a);
		vertices[id].position.z += len * sin(a) * cos(a2);

		vertices[id].position.w = 0.f;
		vertices[id].texture_index = config->start_texture + (int)floor(random1((float)(config->num_textures-0.1), false));

		particles[id].wind_influence = config->avg_wind_influence + random1(config->wind_influence_var, true);
		particles[id].gravity_influence = config->avg_gravity_influence + random1(config->gravity_influence_var, true);

		particles[id].velocity = config->avg_spawn_velocity.xyz + random3(config->spawn_velocity_var.xyz, true);
		particles[id].org_ttl = particles[id].ttl = config->avg_ttl + random1(config->ttl_var, true);
		particles[id].rotation_speed = config->avg_rotation_speed + random1(config->rotation_speed_var, true);
		particles[id].initial_scale = config->avg_scale + random1(config->scale_var, true);
		particles[id].final_scale = particles[id].initial_scale + config->avg_scale_change + random1(config->scale_change_var, true);
		particles[id].dead = 0;
	}
}

