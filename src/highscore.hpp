#ifndef HIGHSCORE_HPP
#define HIGHSCORE_HPP

#include <string>
#include <list>

class Highscore {
	public:
		Highscore(const std::string &file_, unsigned int num_entries);
		~Highscore();

		const std::list<int> &get_entries() const;
		void add_entry(int e);

	private:
		const std::string filename;
		std::list<int> entries;

		void write();
		void load();

};

#endif
