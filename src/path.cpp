#include "path.hpp"

#include <glm/gtx/spline.hpp>
#include <glm/gtx/string_cast.hpp>

Path::Path(const std::vector<glm::vec3> &path) {
	if(path.size() < 4) {
		fprintf(stderr, "Path must contain at least four entries\n");
		abort();
	}

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
			if( it - 2 < path.begin() ) p[0] = & ( * (path.end() - 1) );
			else p[0] = &(* (it - 2) );

			p[1] = &(* (it - 1) );
			p[2] = &(* (it) );

			if(path.end() - it == 1) p[3] = &( *( path.begin() ) );
			else p[3] = &(* (it + 1) );

			glm::vec3 middle_point = glm::catmullRom(*p[0], *p[1], *p[2], *p[3], 0.5f);
			path_length += glm::distance(*p[1], middle_point) + glm::distance(middle_point, *p[2]);
			kp.path_point = path_length;

		}
		printf("Added keypoint[%d]: {%f, %s}\n", kp.index, kp.path_point, glm::to_string(kp.position).c_str());
		points.push_back(kp);
	}
	//Calculate the length from last node to the first node 
	const glm::vec3 *p[4] = {
		&(*(path.end() - 2)), 
		&(*(path.end() - 1)),
		&(*path.begin()),
		&(*(path.begin() + 1))
	};


	glm::vec3 middle_point = glm::catmullRom(*p[0], *p[1], *p[2], *p[3], 0.5f);
	path_length += glm::distance(*p[1], middle_point) + glm::distance(middle_point, *p[2]);
	
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

/**
 * Finds the keypoint (before) for the give normalized position
 */
const Path::keypoint_t &Path::find_keypoint(float position) const {
	for(auto it = points.begin() + 1; it != points.end(); ++it) {
		if(it->path_point > position) {
			return *(it - 1);
		}
	}
	return points.back(); // There are no keypoints after position
}

float Path::normalize_position(float pos) const {
	while(pos < 0.f) pos += path_length;
	return fmod(pos, path_length);
}

/**
 * Use this to initialize a point_t. 
 */
void Path::begin(Path::point_t * point, float start_position) const {
	point->position = glm::vec3(0.f);
	point->path_position = start_position - 1.f;
	point->keypoint = 0;
	next(point, 0.5f); //Now point contains correct data for start - 0.5f, besides direction
	next(point, 0.5f); //Now point contains correct data for start, including direction
}

/**
 * Use this to update the point to a relative position)
 */
void Path::next(Path::point_t * point, float incr) const {
	point->path_position = normalize_position(point->path_position + incr);

	const keypoint_t &kp = find_keypoint(point->path_position);

	const keypoint_t &p0 = keypoint(kp.index - 1);
	const keypoint_t &p1 = kp;
	const keypoint_t &p2 = keypoint(kp.index + 1);
	const keypoint_t &p3 = keypoint(kp.index + 2);

	float s = (point->path_position - p1.path_point) / (p2.path_point - p1.path_point);

	glm::vec3 new_position = glm::catmullRom(p0.position, p1.position, p2.position, p3.position, s);

	point->direction = new_position - point->position;
	point->position = new_position;
	point->keypoint = kp.index;
}
