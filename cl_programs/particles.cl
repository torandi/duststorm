typedef struct {
   float3 direction;
   float ttl;
   float speed;
   float acc;
} particle_t;


typedef struct {

   float4 birth_color; 
   float4 death_color;

   float4 motion_rand; 

   float4 spawn_direction;
   float4 direction_var;

   float4 spawn_position;
   float4 spawn_area;

   //Time to live
   float avg_ttl;
   float ttl_var;
   //Spawn speed
   float avg_spawn_speed; 
   float spawn_speed_var;

   //Acceleration 
   float avg_acc;
   float acc_var;
   //Scale
   float avg_scale;
   float scale_var;
} config_t;

__kernel void run_particles (
                  __global float4 * position, //Shared with GL
                  __global float4 * color, //Shared with GL
                  __global particle_t * particle, //Private to CL
                  __constant config_t * config, //Configuration for the system
                  int particle_limit,
                  double dt) {
    
}
