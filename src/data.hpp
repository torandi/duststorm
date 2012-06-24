#ifndef DATA_HPP
#define DATA_HPP
#include <cstdio>
#include <string>
/*
 * This is intended as the single point of i/o, so as to make it easy to
 * change data reading method (ex to a in-exec type)
 */
class Data {
	public:
		static Data * open(const char * filename);
		static Data * open(const std::string &filename);

		/*
		 * Returns a pointer to the data
		 */
		const void * data() const;

		/*
		 * Returns the total size of the data
		 */
		const size_t &size() const;


		/*
		 * Non-static access functions:
		 * Same behaviour as f{read,seek,tell}
		 */
		size_t read ( void * ptr, size_t size, size_t count) const;
		int seek ( long int offset, int origin ) const;
		long int tell() const;


		/*
		 * Static access: 
		 * (same signatur and behaviour as f{read,seek,tell} except Data * data instead of FILE * file )
		 */
		static size_t read ( void * ptr, size_t size, size_t count, const Data * data);
		static int seek ( const Data * data, long int offset, int origin );
		static long int tell(const Data * data);

		~Data();
	private:
		Data(void * data, const size_t &size);

		void * _data;
		const size_t _size;
		mutable const void * _pos;

		/**
		 * Returns a pointer to the data of the file and sets size to the size read
		 * Returns NULL if it failed
		 *
		 * Should point to a actual function that loads the data (from file or memory)
		 */
		typedef void * file_load_func(const char * filename, size_t &size);
		static file_load_func * load_file;

		static void * load_from_file(const char * filename, size_t &size);

};

#endif
