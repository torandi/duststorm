#include "quad.hpp"
#include "mesh.hpp"

const float Quad::vertices[][5] = { /* x,y,z,u,v */
   {0.f, 0.f, 0.f, 0.f, 0.f},
   {1.f, 0.f, 0.f, 1.f, 0.f},
   {1.f, 1.f, 0.f, 1.f, 1.f},
   {0.f, 1.f, 0.f, 0.f, 1.f},
};

const unsigned int Quad::indices[6] = {0,1,2,0,2,3};

Quad::Quad(bool normal, bool tangent_and_bitangent) : Mesh() {
   set_vertices(vertices, 4);
   set_indices(std::vector<unsigned int>(indices, indices+6));

   if(normal) generate_normals();
   if(tangent_and_bitangent) generate_tangents_and_bitangents();
   generate_vbos();

}
