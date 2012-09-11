#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "movable_object.hpp"

class Mesh : public MovableObject {
	public:


		struct vertex_t {
			glm::vec3 position;
			glm::vec2 tex_coord;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
		};

		Mesh();
		Mesh(const std::vector<vertex_t> &vertices, const std::vector<unsigned int> &indices);
		virtual ~Mesh();

		// Call these methods to activate these features.
		// Automaticaly activated if the corresponding activate function is called
		// Note though that the data is still uploaded to the GPU, just not used
		void activate_normals();
		void activate_tangents_and_bitangents();

		void set_vertices(const std::vector<vertex_t> &vertices);
		void set_indices(const std::vector<unsigned int> &indices);

		//Loads vertex data from array of floats (x, y, z, u, v)
		void set_vertices(const float vertices[][5], const size_t num_vertices);
		void generate_normals();
		void generate_tangents_and_bitangents();
		void ortonormalize_tangent_space();
		//The mesh becommes immutable when vbos have been generated
		void generate_vbos();
		virtual void render(const glm::mat4& m = glm::mat4());
		virtual void render_geometry(const glm::mat4& m = glm::mat4());
		unsigned long num_faces() { return num_faces_; };

	protected:
		std::vector<vertex_t> vertices_;
		std::vector<unsigned int> indices_;
	private:
		GLenum buffers_[2]; //0:vertex buffer, 1: index buffer
		bool vbos_generated_, has_normals_, has_tangents_;
		unsigned long num_faces_;
		glm::vec3 scale_;

		void verify_immutable(const char * where); //Checks that vbos_generated == false


};

#endif
