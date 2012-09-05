#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "platform.h"

#include "utils.hpp"
#include "globals.hpp"
#include "data.hpp"
#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#ifdef HAVE_SYS_TIME_H
#	include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#ifdef WIN32
#	include <Windows.h>
#endif

unsigned long util_utime(){
#ifdef HAVE_GETTIMEOFDAY
	struct timeval cur;
	gettimeofday(&cur, NULL);
	return (unsigned long)(cur.tv_sec * 1000000 + cur.tv_usec);
#elif defined(WIN32)
	static int initialized = 0;
	static int dst_warning = 0;
	static long long int divider = 0;
	static LARGE_INTEGER qpc_freq;

	/* verify timer precision */
	if ( !initialized ){
		QueryPerformanceFrequency(&qpc_freq);
		if ( qpc_freq.QuadPart < 1000000 ){
			fprintf(stderr, "warning: gettimeofday() requires µs precision but is not available (have %lld ticks per second).\n", qpc_freq.QuadPart);
		}

		/* set divider to calculate µs */
		divider = qpc_freq.QuadPart / 1000000;
		initialized = 1;
	}

	/* time query */
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return (unsigned long)(tmp.QuadPart / divider);
#else
#	error util_utime() is not defined for this platform.
#endif
}

void util_usleep(unsigned long wait){
#ifdef HAVE_USLEEP
	usleep(wait);
#elif defined(WIN32)
	Sleep(wait / 1000); /** @todo Sleep only has ms-precision (and bad such). */
#else
#	error util_usleep() is not defined for this platform.
#endif
}

int checkForGLErrors( const char *s ) {
	int errors = 0 ;

	while ( true ) {
		GLenum x = glGetError() ;

		if ( x == GL_NO_ERROR )
			return errors ;

		fprintf( stderr, "%s: OpenGL error: %s\n", s, gluErrorString ( x )) ;
		errors++ ;
	}
	return errors;
}

float radians_to_degrees(double rad) {
   return (float) (rad * (180/M_PI));
}

glm::vec2 screen_pos(const glm::vec2& v, const glm::vec2& size){
	const glm::vec2 w = glm::clamp(v, 0.0f, 1.0f);
	const glm::vec2 delta = glm::vec2(resolution.x, resolution.y) - size;
	return w * delta;
}

void print_mat4(const glm::mat4 &m) {
   printf(" %f %f %f %f \n%f %f %f %f \n%f %f %f %f \n%f %f %f %f \n",
         m[0][0], m[0][1], m[0][2], m[0][3] ,
         m[1][0], m[1][1], m[1][2], m[1][3] ,
         m[2][0], m[2][1], m[2][2], m[2][3] ,
         m[3][0], m[3][1], m[3][2], m[3][3]);
}

bool file_exists(const std::string& filename){
#ifdef HAVE_ACCESS
	return access(filename.c_str(), R_OK) == 0;
#elif defined(WIN32)
	const DWORD dwAttrib = GetFileAttributes(filename.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
	        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
#	error file_exists is not defined for this platform.
#endif
}
