#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils.hpp"
#include <GL/glew.h>
#include <cstdio>
#include <sys/time.h>

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

void print_mat4(const glm::mat4 &m) {
   printf(" %f %f %f %f \n%f %f %f %f \n%f %f %f %f \n%f %f %f %f \n",
         m[0][0], m[0][1], m[0][2], m[0][3] ,
         m[1][0], m[1][1], m[1][2], m[1][3] ,
         m[2][0], m[2][1], m[2][2], m[2][3] ,
         m[3][0], m[3][1], m[3][2], m[3][3]);
}

bool file_exists(const std::string& filename){
	return access(filename.c_str(), R_OK) == 0;
}
