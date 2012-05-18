#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils.hpp"
#include <GL/glew.h>
#include <cstdio>

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
