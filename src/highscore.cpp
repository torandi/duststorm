#include "highscore.hpp"
#include "data.hpp"

Highscore::Highscore(const std::string &file_, unsigned int num_entries) 
	: file(file_)
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
			return;
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
	FILE * f = fopen(file.c_str(), "w");
	for(const int &e : entries) {
		fprintf(f, "%d\n", e);
	}
	fclose(f);
}

void Highscore::load() {
	Data * d = Data::open(file);
	if(d != nullptr) {
		char * line;
		size_t bytes;
		for(int &e : entries) {
			d->getline(&line, &bytes);
			sscanf(line, "%d", &e);
		}
		delete line;
		delete d;
	}
}
