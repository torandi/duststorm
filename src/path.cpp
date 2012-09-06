#include "path.hpp"

#include <list>
#include <glm/gtx/spline.hpp>
#include <glm/gtx/string_cast.hpp>

static const float min_keypoint_distance = 0.1f;
static const float max_keypoint_distance = 10.f;

void Path::optimize_vector(std::vector<glm::vec3> &path) {
	for(auto it = path.begin(); it != path.end(); ++it) {
		auto next = it + 1;
		if(next == path.end()) next = path.begin();

		if(glm::distance(*it, *next) < min_keypoint_distance) {
			it = path.erase(it);
			next = it + 1;
			if(next >= path.end()) break;
		}

		while(glm::distance(*it, *next) > max_keypoint_distance) {
			next = path.insert(next, *it + (*next - *it) / 2.f);
			it = next - 1;
		}
	}
}

Path::Path(const std::vector<glm::vec3> &in_path, bool optimize) {
	if(in_path.size() < 4) {
		fprintf(stderr, "Path must contain at least four entries\n");
		abort();
	}
	std::vector<glm::vec3> opt_path;

	if(optimize) {
		opt_path = in_path;
		optimize_vector(opt_path);
	}
	const std::vector<glm::vec3> &path = optimize ? opt_path : in_path;


	path_length = 0.f;

	for(auto it = path.begin(); it != path.end(); ++it) {
		keypoint_t kp;
		kp.path_point = path_length;
		kp.position = *it;
		kp.index = points.size();

		const glm::vec3 * p[4];

		/* We approximate the path_length of the subpath by
		 * taking the middle point and taking the 
		 * path_length from that to the two points
		 */
		if( it == path.begin() ) p[0] = & ( path.back() );
		else p[0] = &(* (it - 1) );

		p[1] = &( *it );

		if(it + 1 == path.end()) p[2] = &path.front();
		else p[2] = &( *(it + 1 ) );

		if(it + 1 == path.end()) p[3] = &path.front();
		else if(it + 2 == path.end() ) p[3] = &( *(path.begin() + 1) );
		else p[3] = &( *(it + 2 ) );


		glm::vec3 middle_point = glm::catmullRom(*p[0], *p[1], *p[2], *p[3], 0.5f);
		path_length += glm::distance(*p[1], middle_point) + glm::distance(middle_point, *p[2]);

	//	printf("Added keypoint[%d]: {%f, %s}\n", kp.index, kp.path_point, glm::to_string(kp.position).c_str());
		points.push_back(kp);
	}
	
}

float Path::length() const { return path_length; }

const Path::keypoint_t &Path::keypoint(int index) const {
	if(index < 0) return keypoint(points.size() + index);
	return points[index % points.size()];
}

float Path::distance_to_next(unsigned int from) const {
	if(from+1 < points.size()) { 	
		return points[from+1].path_point - points[from].path_point;
	} else {
		return path_length - points[from].path_point;
	}
}

/**
 * Finds the keypoint just before the give normalized position
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
 * Get the 3d coordinate for a given position in the path
 */
glm::vec3 Path::at(float position) const {
	position = normalize_position(position);

	const keypoint_t &kp = find_keypoint(position);

	const keypoint_t &p0 = keypoint(kp.index - 1);
	const keypoint_t &p1 = kp;
	const keypoint_t &p2 = keypoint(kp.index + 1);
	const keypoint_t &p3 = keypoint(kp.index + 2);

	float s = glm::clamp((position - p1.path_point) / distance_to_next(p1.index), 0.f, 1.f);;

	return glm::catmullRom(p0.position, p1.position, p2.position, p3.position, s);
}
