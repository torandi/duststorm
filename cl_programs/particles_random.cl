float random(uint *time, uint id, __global const float * rnd, int max_num_particles) {
	int i = (int)((*time)+id) % max_num_particles;
	float r = rnd[i];
	//Change time to not get same value next call:
	*time += (uint)floor(rnd[*time%max_num_particles]*1000.0);
	return r;
}

//Set dual to true to get a number in range -m..m (otherwise 0..m)
float _random1(float m, bool dual, uint *time, uint id, __global const float * rnd, int max_num_particles) {
	return random(time, id, rnd, max_num_particles)*m*(1+dual) - m*dual;
}

float4 _random4(float4 m, bool dual, uint *time, uint id, __global const float * rnd, int max_num_particles) {
	return (float4) (
								 _random1(m.x, dual, time, id, rnd, max_num_particles),
								 _random1(m.y, dual, time, id, rnd, max_num_particles),
								 _random1(m.z, dual, time, id, rnd, max_num_particles),
								 _random1(m.w, dual, time, id, rnd, max_num_particles)
								 );
}

float3 _random3(float3 m, bool dual, uint *time, int id, __global const float * rnd, int max_num_particles) {
	return (float3) (
								 _random1(m.x, dual, time, id, rnd, max_num_particles),
								 _random1(m.y, dual, time, id, rnd, max_num_particles),
								 _random1(m.z, dual, time, id, rnd, max_num_particles)
								 );
}

#define random1(m, dual) _random1((m), (dual), &time, id, rnd, config->max_num_particles)
#define random3(m, dual) _random3((m), (dual), &time, id, rnd, config->max_num_particles)
#define random4(m, dual) _random4((m), (dual), &time, id, rnd, config->max_num_particles)


