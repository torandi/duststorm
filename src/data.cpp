#include "data.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

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

bool Data::eof() const {
	return ((char*)_pos >= ((char*)_data+_size));
}

const void * Data::data() const {
	return _data;
}

const size_t &Data::size() const {
	return _size;
}

size_t Data::read(void * ptr, size_t size, size_t count) const {
	size_t to_read = count;
	if(((char*)_pos + size*count) > ((char*)_data + _size)) {
		to_read = (_size - ((char*)_pos-(char*)_data)) % size;
	}
	memcpy(ptr, _pos, size*to_read);
	_pos = (void*)((char*)_pos + size*to_read);
	return to_read;
}

int Data::seek(long int offset, int origin) const {
	void * newpos;
	switch(origin) {
		case SEEK_SET:
			newpos = (void*) ((char*)_data + offset);
			break;
		case SEEK_CUR:
			newpos = (void*)((char*)_pos + offset);
			break;
		case SEEK_END:
			newpos = (void*)((char*)_data + _size + offset);
			break;
		default:
			fprintf(stderr, "Warning unknown origin to Data::seek\n");
			return -1;
	}
	if(newpos > ((char*)_data + _size)) {
		return -1;
	} else {
		_pos = newpos;
		return 0;
	}
}

long int Data::tell() const {
	return (long int)((char*)_pos - (char*)_data);
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
