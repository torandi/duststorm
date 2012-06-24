#include "data.hpp"
#include <cstdio>
#include <cstdlib>

Data::file_load_func * Data::load_file = &Data::load_from_file;

Data * Data::open(const std::string &filename) {
	return open(filename.c_str());
}

Data * Data::open(const char * filename) {
	size_t size;
	void * data = load_file(filename, size);
	if(data == nullptr)
		return nullptr;

	return new Data(data, size);
}

void * Data::load_from_file(const char * filename, size_t &size) {
	FILE * file = fopen(filename, "r");
	void * data;
	if(file == nullptr)
		return nullptr;

	// obtain file size:
	fseek (file , 0 , SEEK_END);
	size = ftell (file);
	rewind (file);

	data = malloc(size);
	size_t res = fread(data, 1, size, file);
	if(res != size) {
		fprintf(stderr, "Error in file read: read size was not the expected size (read %lu bytes, expected %lu)\n", res, size);
		abort();
	}
	fclose(file);
	
	return data;
}

const void * Data::data() const {
	return _data;
}

const size_t &Data::size() const {
	return _size;
}

Data::Data(void * data, const size_t &size) :
		_data(data)
	,	_size(size)
	,	_pos(data) {
}

Data::~Data() {
	free(_data);
}

std::ostream& operator<< (std::ostream& out, const Data &data) {
	out.write((const char*)data.data(), (data.size()*sizeof(char)));
	return out;
}

std::ostream& operator<< (std::ostream& out, const Data * data) {
	return (out << *data);
}
