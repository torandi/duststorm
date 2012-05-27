#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

class TimeTable {
protected:
	int read_file(const std::string& filename);
	virtual int parse(char* data) = 0;

	TimeTable();
};

class PointTable: public TimeTable {
public:
	PointTable(const std::string& filename);

	/**
	 * Get position at time t. Interpolated with catmull rom.
	 */
	glm::vec3 at(float t);

protected:
	virtual int parse(char* data);

private:
	struct entry {
		float t;
		glm::vec3 p;
	};

	std::vector<entry> p;
};

#endif /* TIMETABLE_H */
