#ifndef cl_khr_fp64
	#define M_PI 3.14159
#endif

typedef struct particle_t {
	float3 velocity;
	
	float ttl;
	float rotation_speed;
	float initial_scale;
	float final_scale;

	float wind_influence;
	float gravity_influence;
	float org_ttl; //original time to live, stored to get a percentage
	int dead;

	float4 birth_color;
	float4 death_color;
} particle_t __attribute__ ((aligned (16))) ;

typedef struct vertex_t {
	float4 position;
	float4 color;
	float scale;
	int texture_index;
} vertex_t __attribute__ ((aligned (16))) ;

typedef struct config_t {
	float avg_ttl;
	float ttl_var;
	float avg_scale;
	float scale_var;

	float avg_scale_change;
	float scale_change_var;
	float avg_rotation_speed;
	float rotation_speed_var;

	float avg_wind_influence;
	float wind_influence_var;
	float avg_gravity_influence;
	float gravity_influence_var;

	int start_texture;
	int num_textures;
	int max_num_particles;

	float4 spawn_position __attribute__((aligned(16)));
	float4 spawn_area; //The last component specifies radius (will be added to the position with a random angle)

	float4 birth_color; 
	float4 death_color;

	float4 motion_rand; 

	float4 avg_spawn_velocity;
	float4 spawn_velocity_var;

	float4 wind_velocity;	//Speed
	float4 gravity;			//Acceleration

} config_t __attribute__ ((aligned (16))) ;
