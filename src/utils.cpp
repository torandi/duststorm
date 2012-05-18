#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils.hpp"
#include <GL/glew.h>
#include <cstdio>
#include <sys/time.h>

extern struct timeval time;       /* current time */

float get_time() {
	return (float)time.tv_sec + (float)time.tv_usec / 1000000;
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
