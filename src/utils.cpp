#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils.hpp"
#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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

int timetable_parse(const std::string& filename, std::function<void(const std::string&, float, float)> func){
	const char* tablename = PATH_SRC "timetable.txt";
	FILE* timetable = fopen(tablename, "r");
	if ( !timetable ){
		return errno;
	}

	char* line = nullptr;
	size_t size;
	unsigned int linenum = 0;
	while ( getline(&line, &size, timetable) != -1 ){
		const size_t len = strlen(line);
		linenum++;

		/* remove leading and trailing whitespace */
		char* tmp = strdup(line);
		char* entry = tmp;
		if ( entry[len-1] == '\n' ){
			entry[len-1] = 0;
		}
		while ( *entry != 0 && isblank(*entry) ) entry++;

		/* ignore comments and blank lines */
		if ( *entry == '#' || strlen(entry) == 0 ){
			free(tmp);
			continue;
		}

		/* parse line */
		char* name = strtok(entry, ":");
		char* begin = strtok(NULL, ":");
		char* end = strtok(NULL, ":");
		if ( !(name && begin && end) ){
			fprintf(stderr, "%s:%d: malformed entry: \"%.*s\"\n", tablename, linenum, (int)(len-1), line);
			free(tmp);
			continue;
		}
		func(std::string(name), atof(begin), atof(end));
		free(tmp);
	}
	if ( ferror(timetable) ){
		return errno;
	}
	fclose(timetable);
	free(line);

	return 0;
}
