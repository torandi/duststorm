#ifndef PATH_HPP
#define PATH_HPP

#include <glm/glm.hpp>

#include <vector>

/*
 * A path of positions in 3d space with the ability to specify any
 * point on the path in a lineary progression
 */
class Path {
	public:
		Path(const std::vector<glm::vec3> &path);

		glm::vec3 at(float position) const;

		float length() const;

		struct keypoint_t {
			unsigned int index;
			float path_point; /* A number indicating where on the path this keypoint exists */
			glm::vec3 position; /* The position in 3d space */
		};

		std::vector<keypoint_t> points;
	private:
		float path_length;

		const keypoint_t &keypoint(unsigned int index) const;

		float normalize_position(float pos) const;

		/**
		 * Distance from one keypoint to the next
		 */
		float distance_to_next(unsigned int from) const;

		const keypoint_t &find_keypoint(float position) const;
};

#endif
