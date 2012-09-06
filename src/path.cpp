#include "path.hpp"

Path::Path(const std::vector<glm::vec3> &path) {
	path_length = 0;
	for(auto it = path.begin(); it != path.end(); ++it) {
		keypoint_t kp;
		kp.position = *it;
		kp.index = points.size();
		if(it == path.begin()) {
			kp.path_point = 0.f;
		} else {
			/* We approximate the path_length of the subpath by
			 * taking the middle point and taking the 
			 * path_length from that to the two points
			 */
			const glm::vec3 * p[4];
			p[0] = &(* (it - 1) );
			p[1] = &(*it);
			if(path.end() - it > 2) { 
				p[2] = &(* (it + 1) );
				p[3] = &(* (it + 2) );
			} else {
				if(path.end() - it == 1) {
					p[2] = &( *( path.begin() ) );
				} else {
					p[2] = &(* (it + 1) );
				}
				/* Get the first or the second first,
				 * it - path.end() is -1 for the last and -2 for the 
				 * next last one, adding 2 to that gives +1 for the last one and
				 * + 0 for the second last one, yielding the first or second first
				 * entry
				 */
				const size_t index = (2 + it - path.end());
				p[3] = &( *( path.begin() + index ) );
			}
			glm::vec3 middle_point = glm::catmullRom(*p[0], *p[1], *p[2], *p[3], 0.5f);

			path_length += glm::distance(*p[1], middle_point) + glm::distance(middle_point, *p[2]);
			kp.path_point = path_length;

		}
		points.push_back(kp);
	}
}

float Path::length() const { return path_length; }

const Path::keypoint_t &Path::keypoint(unsigned int index) const {
	if(index < 0) return keypoint(points.size() - index);
	return points[index % points.size()];
}

float Path::distance_to_next(unsigned int from) const {
	if(from+1 < points.size()) { 	
		return points[from+1].path_point - points[from].path_point;
	} else {
		return path_length - from;
	}
}
