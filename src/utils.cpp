#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils.hpp"
#include "globals.hpp"
#include "data.hpp"
#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/time.h>
#endif

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
#ifndef WIN32
	return access(filename.c_str(), R_OK) == 0;
#else
	std::wstring stemp = std::wstring(filename.begin(), filename.end());
	DWORD dwAttrib = GetFileAttributes(stemp.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
}

std::string color_to_string(const glm::ivec3 &color) {
	char str[7];
	sprintf(str, "%02x%02x%02x", color.x, color.y, color.z);
	return std::string(str);
}

long get_millis() {
#ifndef WIN32
	struct timeval cur;
	gettimeofday(&cur, NULL);
	return (long) (cur.tv_sec * 1000000 + cur.tv_usec);
#else
	return (long) GetTickCount()*1000;
#endif
}

void sleep_millis(long wait) {
#ifndef WIN32
	usleep(wait);
#else
	Sleep(wait / 1000);
#endif
}
