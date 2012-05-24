typedef struct {
   float3 direction;
   float ttl;
   float speed;
   float acc;
} particle_t ;


typedef struct {

   float4 birth_color; 
   float4 death_color;

   float4 motion_rand; 

   float4 spawn_direction;
   float4 direction_var;

   float4 spawn_position;
   float4 spawn_area;

   float avg_ttl;
   float ttl_var;
   float avg_spawn_speed; 
   float spawn_speed_var;

   float avg_acc;
   float acc_var;
   float avg_scale;
   float scale_var;

	uint max_num_particles;
} config_t ;

__kernel void run_particles (
                  __global float4 * position, 
                  __global float4 * color, 
                  __global particle_t * particle, 
                  __constant config_t * config, 
						__constant float * rnd,
                  uint particle_limit,
                  float dt,
						uint time
						)
{
	int id = get_global_id(0);    
}

float random(uint time, int id, __constant float * rnd, uint max_num_particles) {
	return rnd[(time+id)%max_num_particles];
}

//Set dual to true to get a number in range -m..m (otherwise 0..m)
float random1(float m, bool dual, uint time, int id, __constant float * rnd, uint max_num_particles) {
	return random(time, id, rnd, max_num_particles)*m*(1+dual) - m*dual;
}

float random4(float4 m, bool dual, uint time, int id, __constant float * rnd, uint max_num_particles) {
	return float4(
					random1(m.x, dual, time, id, rnd, max_num_particles),
					random1(m.y, dual, time, id, rnd, max_num_particles),
					random1(m.z, dual, time, id, rnd, max_num_particles),
					random1(m.w, dual, time, id, rnd, max_num_particles)
				);
}
