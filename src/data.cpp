#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "data.hpp"
#include "globals.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static long file_size(FILE* fp){
	const long cur = ftell(fp);
	fseek (fp , 0 , SEEK_END);
	const long bytes = ftell(fp);
	fseek(fp, cur, SEEK_SET);
	return bytes;
}

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
	FILE * file = fopen(filename, "rb");
	if(file == nullptr) {
		fprintf(verbose, "[Data] Couldn't open file `%s'\n", filename);
		return nullptr;
	}

	size = file_size(file);

	data = malloc(size);
	size_t read_bytes = 0;
	while(read_bytes < size) {
		size_t res = fread(data, 1, size, file);
		if(res == 0) break;
		read_bytes += res;
	}

	if(read_bytes != size) {
		fprintf(stderr, "Error in file read: read size was not the expected size (read %lu bytes, expected %lu)\n", read_bytes, size);
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

ssize_t Data::read(void * ptr, size_t size, size_t count) const {
	ssize_t to_read = count;
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
			errno = EINVAL;
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

ssize_t Data::getline(char **lineptr, size_t *n) const{
	if(n == nullptr || lineptr == nullptr) {
		errno = EINVAL;
		return -1;
	}

	if(eof())
		return -1;

	//Find the next newline (or eof)
	size_t next = 0;
	size_t max = _size - ((char*)_pos-(char*)_data); //Bytes left until eof
	while( next < max && *((char*)_pos+next) != '\n' ) { ++next; }
	++next; //Include the newline

	if(*lineptr == nullptr) {
		*lineptr = (char*)malloc(sizeof(char) * (next + 1) );
		*n = (next + 1);
	} else if(*n < (next + 1 )) {
		*lineptr = (char*)realloc(*lineptr, (next + 1));
		*n = (next + 1);
	}

	return read((void*)*lineptr, sizeof(char), next);
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
