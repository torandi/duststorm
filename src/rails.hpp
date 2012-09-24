#ifndef RAILS_HPP
#define RAILS_HPP

#include "path.hpp"
#include "mesh.hpp"

#include <vector>
#include <glm/glm.hpp>

class Rails : public Mesh {
	public:
		Rails(const Path * _path, float step = 1.f);
		virtual ~Rails();

		void render(const glm::mat4 &m = glm::mat4());

		glm::vec3 perpendicular_vector_at(float pos) const;
	private:
		const Path * path;
		Shader * shader;
		/**
		 * The vertex structure is for each slice as follows:
		 * 1 - 2     6 - 5
		 * |   |     |   |
		 *	0   3     7   4
		 *
		 
		 *	@param prev: should contain previous coordinate (for direction calculation)
		 *	and is set to this coordinate when the method returns
		 *
		 *	@return This method returns the index of the first vertex in the slice
		 *	that was generated
		 */
		unsigned int emit_vertices(float position, glm::vec3 &v);

		void generate_indices(float position, glm::vec3 &previous);

		/**
		 * Extra to keep track of perpendicular vector
		 */
		const float step;
		std::vector<glm::vec3> perpendicular_vectors;
};

#endif
