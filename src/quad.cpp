#include "quad.hpp"
#include "mesh.hpp"

#include <cstring>

static const int NUM_VERTICES = 4;
static const int NUM_VERTEX_DATA_POINTS = 5;

const float Quad::vertices[][NUM_VERTEX_DATA_POINTS] = { /* x,y,z,u,v */
	{1.f, 1.f, 0.f, 1.f, 1.f},
	{1.f, 0.f, 0.f, 1.f, 0.f},
	{0.f, 0.f, 0.f, 0.f, 0.f},
	{0.f, 1.f, 0.f, 0.f, 1.f},
};

const unsigned int Quad::indices[] = {0,1,2,3,0,2};

Quad::Quad(glm::vec2 texture_scale, bool normal, bool tangent_and_bitangent) : Mesh() {
	float v[NUM_VERTEX_DATA_POINTS][NUM_VERTEX_DATA_POINTS];
	memcpy(v, vertices, NUM_VERTICES*NUM_VERTEX_DATA_POINTS*sizeof(float));
	for(int i=0; i <NUM_VERTICES; ++i) {
		v[i][3]*=texture_scale.x; //u
		v[i][4]*=texture_scale.y; //v
	}
	set_vertices(v, NUM_VERTICES);
	set_indices(std::vector<unsigned int>(indices, indices+sizeof(indices)));

	if(normal) generate_normals();
	if(tangent_and_bitangent) generate_tangents_and_bitangents();
	generate_vbos();
}
