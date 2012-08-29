#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include "shader.hpp"
#include <functional>
#include <glm/glm.hpp>


/**
 * Get the current in-engine time.
 */
float get_time();

long get_millis();
void sleep_millis(long wait);

int checkForGLErrors( const char *s );

float radians_to_degrees(double rad);

void print_mat4(const glm::mat4 &m);

inline float frand() {
	return (float)rand()/RAND_MAX;
}

std::string color_to_string(const glm::ivec3 &color);

/**
 * Reposition a position onto screen so [0,0] -> [0,0] and [1,1] -> [w-x, h-y].
 * E.g. [0.5, 0.5] will center box.
 */
glm::vec2 screen_pos(const glm::vec2& v, const glm::vec2& size);

/**
 * Test if a filename exists.
 */
bool file_exists(const std::string& filename);

/* Fuck you microsoft: */
#define M_E 2.71828182845904523536
#define M_LOG2E 1.44269504088896340736
#define M_LOG10E 0.434294481903251827651
#define M_LN2 0.693147180559945309417
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.785398163397448309616
#define M_1_PI 0.318309886183790671538
#define M_2_PI 0.636619772367581343076
#define M_1_SQRTPI 0.564189583547756286948
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT_2 0.707106781186547524401

#ifdef WIN32
#define round(val) floor((val) + 0.5)
#endif

#endif
