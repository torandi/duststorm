#ifndef cl_khr_fp64
	#define M_PI 3.14159
#endif

typedef struct particle_t {
	float3 direction;
	
	float ttl;
	float speed;
	float acc;
	float rotation_speed;

	float initial_scale;
	float final_scale;
	float org_ttl; //original time to live, stored to get a percentage
	int dead;
} particle_t __attribute__ ((aligned (16))) ;

typedef struct vertex_t {
	float4 position;
	float4 color;
	float scale;
	int texture_index;
} vertex_t __attribute__ ((aligned (16))) ;

typedef struct config_t {

	float4 birth_color; 
	float4 death_color;

	float3 motion_rand; 

	float3 spawn_direction;
	float3 direction_var;

	float3 spawn_position;
	float4 spawn_area; //The last component specifies radius (will be added to the position with a random angle)

	float3 directional_speed;
	float3 directional_speed_var;

	float avg_ttl;
	float ttl_var;
	float avg_spawn_speed; 
	float spawn_speed_var;

	float avg_acc;
	float acc_var;
	float avg_scale;
	float scale_var;

	float avg_scale_change;
	float scale_change_var;

	float avg_rotation_speed;
	float rotation_speed_var;

	int num_textures;
	int max_num_particles;

} config_t __attribute__ ((aligned (16))) ;
