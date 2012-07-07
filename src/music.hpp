#ifndef MUSIC_HPP
#define MUSIC_HPP

#include "data.hpp"

#include <fmodex/fmod.hpp>

/**
 * A music player
 */
class Music {
	public:
		/*
		 * Create a music player for the selected file.
		 * Optional: Specify buffer size in bytes
		 */
		Music(const char * file);
		~Music();

		bool is_playing() const;
		/**
		 * Start playing the file.
		 */
		void play();
		/*
		 * Stop the playback
		 */
		void stop();
		/**
		 * Returns the time since playback started or -1 if it is not available
		 */
		double time() const;

		/**
		 * Seeks to the given time
		 */
		void seek(double t);

	private:
		static FMOD::System * system_;
		static unsigned int system_usage_; //usage count

		static void initialize_fmod();
		static void terminate_fmod();

		static void errcheck(const char * contex);
		
		static FMOD_RESULT result_;

		Data * source;
		FMOD::Sound * sound_;
		FMOD::Channel * channel_;
	
		double start_time;
};
#endif
