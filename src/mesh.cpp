#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.hpp"
#include "utils.hpp"

#include <GL/glew.h>

#include <cstdio>
#include <glm/glm.hpp>
#include <vector>
#include <cassert>

Mesh::Mesh() : MovableObject(), vbos_generated_(false), has_tangents_(false), scale_(1.f), scale_matrix_dirty_(true)	{ }

Mesh::Mesh(const std::vector<vertex_t> &vertices, const std::vector<unsigned int> &indices) :
	MovableObject(), 
	vbos_generated_(false),has_tangents_(false), scale_(1.f), vertices_(vertices), indices_(indices), scale_matrix_dirty_(true){
	assert((indices.size()%3)==0);
}

Mesh::~Mesh() {
	if(vbos_generated_)
		glDeleteBuffers(2, buffers_);
}

void Mesh::set_vertices(const std::vector<vertex_t> &vertices) {
   verify_immutable("set_vertices");
   vertices_ = vertices;
}

void Mesh::set_indices(const std::vector<unsigned int> &indices) {
   verify_immutable("set_indices");
   indices_ = indices;
}

void Mesh::set_vertices(const float vertices[][5], const size_t num_vertices) {
   verify_immutable("set_vertices");
   vertices_.clear();
   vertex_t v;

   v.normal = glm::vec3();
   v.tangent = glm::vec3();
   v.bitangent = glm::vec3();

   for(unsigned int i=0; i < num_vertices; ++i) {
      v.position = glm::vec3(vertices[i][0], vertices[i][1], vertices[i][2]);
      v.tex_coord = glm::vec2(vertices[i][3], vertices[i][4]);
      vertices_.push_back(v);
   }
}

void Mesh::generate_normals() {
   if(vertices_.size() == 3 || indices_.size() == 0) {
      fprintf(stderr, "Mesh::generate_normals() called with vertices or indices empty\n");
      abort();
   }

	verify_immutable("calculate_normals()");

	for(unsigned int i=0; i<indices_.size(); i+=3) {
		unsigned int tri[3] = {indices_[i], indices_[i+1], indices_[i+2]};
		vertex_t * face[3] = {&vertices_[tri[0]], &vertices_[tri[1]], &vertices_[tri[2]]};
		glm::vec3 v1 = face[1]->position-face[0]->position;
		glm::vec3 v2 = face[2]->position-face[0]->position;
		glm::vec3 normal = glm::cross(v1, v2);
		for(int f=0; f<3;++f) {
			face[f]->normal += normal;
		}
	}
   has_normals_ = true;
}

void Mesh::activate_normals() {
   has_normals_ = true;
}

void Mesh::activate_tangents_and_bitangents() {
   has_tangents_ = true;
}


//This method orgonormalizes the tangent space
void Mesh::ortonormalize_tangent_space() {
   if(! (has_normals_ && has_tangents_)) {
      fprintf(stderr, "Mesh::ortonormalize_tangent_space() called with normals or tangents inactive\n");
      abort();
   }

	for(std::vector<vertex_t>::iterator it=vertices_.begin(); it!=vertices_.end(); ++it) {
		it->normal = glm::normalize(it->normal);	
		//Make sure tangent is ortogonal to normal (and normalized)
		it->tangent = glm::normalize(it->tangent - it->normal*glm::dot(it->normal, it->tangent));
		//Normalize bitangent
		it->bitangent = glm::normalize(it->bitangent);
		//Make sure tangent space is right handed:
		glm::vec3 new_bitangent = glm::cross(it->normal, it->tangent);
		if(glm::dot(glm::cross(it->normal, it->tangent), new_bitangent) < 0.0f) {
			it->tangent *= -1.0f;
		}
		it->bitangent = new_bitangent;

	}
}

void Mesh::generate_tangents_and_bitangents() {
   if(vertices_.size() == 3 || indices_.size() == 0) {
      fprintf(stderr, "Mesh::generate_tangents_and_bitangents() called with vertices or indices empty\n");
      abort();
   }

	for(unsigned int i=0; i<indices_.size(); i+=3) {
		unsigned int tri[3] = {indices_[i], indices_[i+1], indices_[i+2]};
		vertex_t * face[3] = {&vertices_[tri[0]], &vertices_[tri[1]], &vertices_[tri[2]]};
		glm::vec3 v1 = face[1]->position-face[0]->position;
		glm::vec3 v2 = face[2]->position-face[0]->position;
		glm::vec2 uv1 = face[1]->tex_coord-face[0]->tex_coord;
		glm::vec2 uv2 = face[2]->tex_coord-face[0]->tex_coord;

		float r=1.f / (uv1.x * uv2.y - uv1.y * uv2.x);
		glm::vec3 tangent = (v1 * uv2.y - v2 * uv1.y)*r;
		glm::vec3 bitangent = (v2 * uv1.x - v1 * uv2.x)*r;

		for(int f=0; f<3; ++f) {
			face[f]->tangent += tangent;
			face[f]->bitangent += bitangent;
		}
	}
   has_tangents_ = true;
}

void Mesh::verify_immutable(const char * where) {
	if(vbos_generated_) {
		fprintf(stderr,"Mesh::%s can not be used after vertex buffers have been generated\n", where);
      abort();
	}
}

void Mesh::generate_vbos() {
	verify_immutable("generate_vbos()");

	//Upload data:
	glGenBuffers(2, buffers_);
	checkForGLErrors("Mesh::generate_vbos(): gen buffers");

	glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t)*vertices_.size(), &vertices_.front(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkForGLErrors("Mesh::generate_vbos(): fill array buffer");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices_.size(), &indices_.front(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	checkForGLErrors("Mesh::generate_vbos(): fill element array buffer");

	num_faces_ = indices_.size();

	vbos_generated_ = true;
}

void Mesh::render() {

	Shader::upload_model_matrix(matrix());

	glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);

	checkForGLErrors("Mesh::render(): Bind buffers");

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)));

   if(has_normals_) {
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (sizeof(glm::vec3)+sizeof(glm::vec2)));
   }

   if(has_tangents_) {
      glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (2*sizeof(glm::vec3)+sizeof(glm::vec2)));
      glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const GLvoid*) (3*sizeof(glm::vec3)+sizeof(glm::vec2)));
   }

	checkForGLErrors("Mesh::render(): Set vertex attribs");

	glDrawElements(GL_TRIANGLES, num_faces_, GL_UNSIGNED_INT, 0);	

	checkForGLErrors("Mesh::render(): glDrawElements()");

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	checkForGLErrors("Mesh::render(): Teardown ");
}

const glm::vec3 &Mesh::scale() const { return scale_; }

void Mesh::set_scale(const float &scale) {
	set_scale(glm::vec3(scale));
}

void Mesh::set_scale(const glm::vec3 &scale) {
	scale_ = scale;
	scale_matrix_dirty_ = true;
}

const glm::mat4 Mesh::matrix() const{
	if(translation_matrix_dirty_ || rotation_matrix_dirty_ || scale_matrix_dirty_) matrix_ = scale_matrix()*translation_matrix()*rotation_matrix();
	return matrix_;
}

const glm::mat4 Mesh::scale_matrix() const {
	if(scale_matrix_dirty_) {
		scale_matrix_ = glm::scale(glm::mat4(1.f), scale_);
		scale_matrix_dirty_ = false;
	}
	return scale_matrix_;
}
