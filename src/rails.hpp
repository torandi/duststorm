#ifndef RAILS_HPP
#define RAILS_HPP

#include "path.hpp"
#include "mesh.hpp"

class Rails : public Mesh {
	public:
		Rails(const Path * _path, float detail = 1.f);
		virtual ~Rails();
	private:
		const Path * path;
		/**
		 * The vertex structure is for each slice as follows:
		 * 1 - 2     5 - 6
		 * |   |     |   |
		 *	0   3     4   7
		 *
		 
		 *	@param prev: should contain previous coordinate (for direction calculation)
		 *	and is set to this coordinate when the method returns
		 *
		 *	@return This method returns the index of the first vertex in the slice
		 *	that was generated
		 */
		unsigned int emit_vertices(float position, glm::vec3 &v);
};

#endif
