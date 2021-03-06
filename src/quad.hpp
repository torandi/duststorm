#ifndef QUAD_H
#define QUAD_H

#include "mesh.hpp"

class Quad : public Mesh {
   public:

	Quad(glm::vec2 texture_scale = glm::vec2(1.0f, -1.0f), bool normal=true, bool tangent_and_bitangent=false);
   private:
      static const float vertices[][5];
      static const unsigned int indices[6];
};

#endif
