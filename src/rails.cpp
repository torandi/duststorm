#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rails.hpp"
#include "shader.hpp"

#include "terrain.hpp"

#include "globals.hpp"

#include <cstdio>

#include <glm/gtx/string_cast.hpp>

/**
 * Configuration
 * Height: height of the rails
 * width: width of a single rail
 * separation: width between the rails
 * uv_offset: how long a single texture stretches
 */
static const float height = 0.15f;
static const float width = 0.10f;
static const float separation = 0.5f;
static const float uv_offset = 1.f;

static const unsigned int slice_indices = 8;

Rails::Rails(const Path * _path, float step) : Mesh(), path(_path){

	shader = Shader::create_shader("normal");

	glm::vec3 previous = path->at(-step);
	emit_vertices(0.f, previous);
	for(float p = step; p < path->length(); p += step) {
		generate_indices(p, previous);
	}

	generate_indices(path->length() + 0.01, previous);

	generate_normals();
	generate_tangents_and_bitangents();
	ortonormalize_tangent_space();
	generate_vbos();
}

Rails::~Rails() { }

/**
 * The vertex structure is for each slice as follows:
 * 1 - 2     6 - 5
 * |   |     |   |
 *	0   3     7   4
 *
 * uv coordinates are (offset = position / uv_offset)
 * (1/3,offset) - (2/3,offset) 
 *   |           |
 * (0,offset)   (1,offset)    
 *
 *	This method returns the index of the first vertex in the slice
 *	that was generated
 */
unsigned int Rails::emit_vertices(float path_position, glm::vec3 &prev) {
	unsigned int start_index = vertices_.size();

	const glm::vec3 initial_normal = glm::vec3(0.f, 1.f, 0.f);

	const glm::vec3 pos = path->at(path_position);
	const glm::vec3 direction = glm::normalize(pos - prev);
	const glm::vec3 side = glm::normalize(glm::cross(direction, initial_normal)); //points right
	const glm::vec3 normal = glm::normalize(glm::cross(side, direction));
/*
	fprintf(verbose, "%f (%s - %s): %s %s %s\n", path_position
							, glm::to_string(prev).c_str()
							, glm::to_string(pos).c_str()
							, glm::to_string(direction).c_str()
							, glm::to_string(side).c_str()
							, glm::to_string(normal).c_str());
*/
	const glm::vec3 right = pos + (side * separation/2.f);
	const glm::vec3 left = pos - (side * separation/2.f);

	vertex_t v;

	float uv_y = path_position / uv_offset;


	//0
	v.position = left - side * width;
	v.tex_coord = glm::vec2(0.f, uv_y);
	vertices_.push_back(v);

	//1
	v.position = left - side * width + normal * height;
	v.tex_coord = glm::vec2(1.f/3.f, uv_y);
	vertices_.push_back(v);

	//2
	v.position = left + normal * height;
	v.tex_coord = glm::vec2(2.f/3.f, uv_y);
	vertices_.push_back(v);

	//3
	v.position = left;
	v.tex_coord = glm::vec2(1.f, uv_y);
	vertices_.push_back(v);

	//--- Second rail
	
	//4
	v.position = right;
	v.tex_coord = glm::vec2(1.f, uv_y);
	vertices_.push_back(v);

	//5
	v.position = right + normal * height;
	v.tex_coord = glm::vec2(2.f/3.f, uv_y);
	vertices_.push_back(v);

	//6
	v.position = right + side * width + normal * height;
	v.tex_coord = glm::vec2(1.f/3.f, uv_y);
	vertices_.push_back(v);

	//7
	v.position = right + side * width;
	v.tex_coord = glm::vec2(0.f, uv_y);
	vertices_.push_back(v);

	prev = pos;

	return start_index;
}

void Rails::generate_indices(float p, glm::vec3 &previous) {
	unsigned int index = emit_vertices(p, previous) - slice_indices; //Build faces between this and the previous slice

	//Build two rails
	for(int i=0; i<2; ++i) {

		//Outer side

		indices_.push_back(index + 8);
		indices_.push_back(index + 0);
		indices_.push_back(index + 1);

		indices_.push_back(index + 8);
		indices_.push_back(index + 1);
		indices_.push_back(index + 9);

		//Upper side

		indices_.push_back(index + 9);
		indices_.push_back(index + 1);
		indices_.push_back(index + 2);

		indices_.push_back(index + 9);
		indices_.push_back(index + 2);
		indices_.push_back(index + 10);

		//Inner side

		indices_.push_back(index + 10);
		indices_.push_back(index + 2);
		indices_.push_back(index + 3);

		indices_.push_back(index + 10);
		indices_.push_back(index + 3);
		indices_.push_back(index + 11);

		index += 4; //The indices in the next rail are offset by 4
	}
}

void Rails::render(const glm::mat4 &m) {
	shader->bind();
	Mesh::render(m);
}
