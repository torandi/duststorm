#include "highscore.hpp"
#include "data.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

Highscore::Highscore(const std::string &file_, unsigned int num_entries) : filename(file_)
{
	for(unsigned int i = 0; i< num_entries; ++i) {
		entries.push_back(-1);
	}
	load();
	write();
}

void Highscore::add_entry(int e) {
	for(auto it = entries.begin(); it != entries.end(); ++it) {
		if(*it < e) {
			entries.insert(it, e);
			entries.pop_back();
			break;
		}
	}
	write();
}


const std::list<int> &Highscore::get_entries() const {
	return entries;
}

Highscore::~Highscore() {
} 

void Highscore::write() {
	FILE * f = fopen(filename.c_str(), "w");
	if(!f) {
		fprintf(stderr, "Failed to open %s: %s\n", filename.c_str(), strerror(errno));
		abort();
	}
	for(const int &e : entries) {
		fprintf(f, "%d\n", e);
	}
	fclose(f);
}

void Highscore::load() {
	Data * d = Data::open(filename);
	if(d != nullptr) {
		char * line = nullptr;
		size_t bytes;
		for(int &e : entries) {
			if(d->getline(&line, &bytes) > -1) {
				sscanf(line, "%d", &e);
			} else {
				e = -1;
			}
		}
		delete line;
		delete d;
	}
}
