#ifndef PATH_HPP
#define PATH_HPP

#include <glm/glm.hpp>

#include <vector>

class Path {
	public:
		Path(const std::vector<glm::vec3> &path);

		struct point_t {
			glm::vec3 position;		/* position in 3d space of the current point */
			glm::vec3 direction;		/* Non-normalized direction (current point - previous point) */
			float path_position;		/* position in the path */
			unsigned int keypoint;	/* keypoint before or on this position */
		};

		/**
		 * Use this to initialize a point_t. 
		 */
		void begin(point_t * point, float start_position=0.f) const;

		/**
		 * Use this to update the point to a relative position)
		 */
		void next(point_t * point, float incr) const;

		float length() const;

	private:
		struct keypoint_t {
			unsigned int index;
			float path_point; /* A number indicating where on the path this keypoint exists */
			glm::vec3 position; /* The position in 3d space */
		};

		std::vector<keypoint_t> points;
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
