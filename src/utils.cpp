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
	return access(filename.c_str(), R_OK) == 0;
}

int timetable_parse(const std::string& filename, std::function<void(const std::string&, float, float)> func){
	const char* tablename = filename.c_str();
	Data * timetable = Data::open(tablename);
	if ( timetable == NULL ){
		return errno;
	}

	char* line = nullptr;
	size_t size;
	unsigned int linenum = 0;
	while ( timetable->getline(&line, &size) != -1 ){
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
	delete timetable;
	free(line);

	return 0;
}

std::string color_to_string(const glm::ivec4 &color) {
	char str[6];
	sprintf(str, "%x%x%x", color.x, color.y, color.z);
	return std::string(str);
}
