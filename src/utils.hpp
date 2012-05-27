#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include "shader.hpp"
#include <functional>

/**
 * Get the current in-engine time.
 */
float get_time();

int checkForGLErrors( const char *s );

float radians_to_degrees(double rad);

void print_mat4(const glm::mat4 &m);

inline float frand() {
	return (float)rand()/RAND_MAX;
}

/**
 * Test if a filename exists.
 */
bool file_exists(const std::string& filename);

/**
 * Parse timetable.
 * Calls func for each valid line.
 * @return 0 if successful and errno on errors.
 */
int timetable_parse(const std::string& filename, std::function<void(const std::string&, float, float)> func);

#endif
