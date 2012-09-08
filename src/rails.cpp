#include "rails.hpp"

#include "terrain.hpp"

#include <cstdio>

/**
 * Configuration
 * Height: height of the rails
 * width: width of a single rail
 * separation: width between the rails
 * uv_offset: how long a single texture stretches
 */
static const float height = 0.25f;
static const float width = 0.10f;
static const float separation = 0.5f;
static const float uv_offset = 5.f;

Rails::Rails(const Path * _path, float detail) : Mesh(), path(_path){
	
}

Rails::~Rails() {

}

/**
 * The vertex structure is for each slice as follows:
 * 1 - 2     5 - 6
 * |   |     |   |
 *	0   3     4   7
 *
 * uv coordinates are (offset = position / uv_offset)
 * (1/3,offset) - (2/3,offset) 
 *   |           |
 * (0,offset)   (1,offset)    
 *
 *	This method returns the index of the first vertex in the slice
 *	that was generated
 */
unsigned int Rails::emit_vertices(float position, glm::vec3 &prev) {
	unsigned int start_index = vertices_.size();

	glm::vec3 initial_normal = glm::vec3(0.f, 1.f, 0.f);

	glm::vec3 pos = path->at(position);
	glm::vec3 direction = glm::normalize(position - prev);
	glm::vec3 side = glm::normalize(glm::cross(initial_normal, direction));
	glm::vec3 normal = glm::normalize(glm::cross(direction, side));

	glm::vec3 left = pos - side * separation/2.f;
	glm::vec3 right = pos + side * separation/2.f;

	vertex_t v;

	float uv_y = position / uv_offset;

	//0
	v.position = left - side * width;
	v.tex_coord = glm::vec2(0.f, uv_y);
	vertices_.push_back(v);

	//1
	v.position += normal * height;
	v.tex_coord = glm::vec2(1.f/3.f, uv_y);
	vertices_.push_back(v);

	//2
	v.position += side * width;
	v.tex_coord = glm::vec2(2.f/3.f, uv_y);
	vertices_.push_back(v);

	//3
	v.position -= normal * height;
	v.tex_coord = glm::vec2(1.f, uv_y);
	vertices_.push_back(v);

	//--- Second rail
	
	//4
	v.position = right;
	v.tex_coord = glm::vec2(1.f, uv_y);
	vertices_.push_back(v);

	//5
	v.position += normal * height;
	v.tex_coord = glm::vec2(2.f/3.f, uv_y);
	vertices_.push_back(v);

	//6
	v.position += side * width;
	v.tex_coord = glm::vec2(1.f/3.f, uv_y);
	vertices_.push_back(v);

	//7
	v.position -= normal * height;
	v.tex_coord = glm::vec2(0.f, uv_y);
	vertices_.push_back(v);

	prev = pos;

	return start_index;
}
