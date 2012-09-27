float random(uint *time, const uint id, __global const float * const rnd, const int max_num_particles) {
	float r = rnd[(int)((*time)+id) % max_num_particles];
	//Change time to not get same value next call:
	(*time)++;
	return r;
}

//Set dual to true to get a number in range -m..m (otherwise 0..m)
float _random1(const float m, const bool dual, uint *time, const uint id, __global const float * const rnd, const int max_num_particles) {
	return random(time, id, rnd, max_num_particles)*m*(1+dual) - m*dual;
}

float4 _random4(const float4 m, const bool dual, uint *time, const uint id, __global const float * const rnd, const int max_num_particles) {
	return (float4) (
								 _random1(m.x, dual, time, id, rnd, max_num_particles),
								 _random1(m.y, dual, time, id, rnd, max_num_particles),
								 _random1(m.z, dual, time, id, rnd, max_num_particles),
								 _random1(m.w, dual, time, id, rnd, max_num_particles)
								 );
}

float3 _random3(const float3 m, const bool dual, uint *time, const int id, __global const float * const rnd, const int max_num_particles) {
	return (float3) (
								 _random1(m.x, dual, time, id, rnd, max_num_particles),
								 _random1(m.y, dual, time, id, rnd, max_num_particles),
								 _random1(m.z, dual, time, id, rnd, max_num_particles)
								 );
}

#define random1(m, dual) _random1((m), (dual), &time, id, rnd, config->max_num_particles)
#define random3(m, dual) _random3((m), (dual), &time, id, rnd, config->max_num_particles)
#define random4(m, dual) _random4((m), (dual), &time, id, rnd, config->max_num_particles)


