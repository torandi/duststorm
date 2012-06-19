#include "quad.hpp"
#include "mesh.hpp"

#include <cstring>

const float Quad::vertices[][5] = { /* x,y,z,u,v */
	{1.f, 1.f, 0.f, 1.f, 1.f},
	{1.f, 0.f, 0.f, 1.f, 0.f},
	{0.f, 0.f, 0.f, 0.f, 0.f},
	{0.f, 1.f, 0.f, 0.f, 1.f},
};

const unsigned int Quad::indices[6] = {0,1,2,3,0,2};

Quad::Quad(glm::vec2 texture_scale, bool normal, bool tangent_and_bitangent) : Mesh() {
	float v[4][5];
	memcpy(v, vertices, 20*sizeof(float));
	for(int i=0; i <5; ++i) {
		v[i][3]*=texture_scale.x;
		v[i][4]*=texture_scale.y;
	}
	set_vertices(v, 4);
	set_indices(std::vector<unsigned int>(indices, indices+6));

	if(normal) generate_normals();
	if(tangent_and_bitangent) generate_tangents_and_bitangents();
	generate_vbos();

}
