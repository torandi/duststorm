#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "timetable.hpp"
#include <cstdlib>
#include <cstring>
#include <glm/gtx/spline.hpp>

TimeTable::TimeTable(){

}

int TimeTable::read_file(const std::string& filename){
	const std::string expanded = PATH_SRC + filename;
	FILE* fp = fopen(expanded.c_str(), "r");
	if ( !fp ){
		return errno;
	}

	char* line = nullptr;
	size_t size;
	unsigned int linenum = 0;
	while ( getline(&line, &size, fp) != -1 ){
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
		if ( parse(entry) != 0 ){
			fprintf(stderr, "%s:%d: malformed entry: \"%.*s\"\n", expanded.c_str(), linenum, (int)(len-1), line);
		}

		free(tmp);
	}
	if ( ferror(fp) ){
		return errno;
	}
	fclose(fp);
	free(line);

	return 0;
}

PointTable::PointTable(const std::string& filename){
	read_file(filename);
}

int PointTable::parse(char* data){
	char* t = strtok(data, ":");
	char* x = strtok(NULL, ",");
	char* y = strtok(NULL, ",");
	char* z = strtok(NULL, ",");
	if ( !(t && x && y && z) ){
		return 1;
	}
	p.push_back(entry({(float)atof(t), glm::vec3(atof(x), atof(y), atof(z))}));
	return 0;
}

glm::vec3 PointTable::at(float t){
	auto cur = p.begin();
	if ( cur == p.end() ) return glm::vec3();
	auto next = cur+1;

	/* find current point */
	while ( next != p.end() && (*next).t < t ){
		cur = next;
		next = cur+1;
	}

	/* at firs/last point */
	if ( next == p.end() ){
		return (*(cur-1)).p;
	} else if ( next+1 == p.end() ){
		return (*cur).p;
	} else if ( cur == p.begin() ){
		return (*next).p;
	}

	const glm::vec3 p0 = (*(cur-1)).p;
	const glm::vec3 p1 = (*cur).p;
	const glm::vec3 p2 = (*next).p;
	const glm::vec3 p3 = (*(next+1)).p;
	const float s = (t-(*cur).t) / ((*next).t - (*cur).t);

	return glm::catmullRom(p0, p1, p2, p3, s);
}
