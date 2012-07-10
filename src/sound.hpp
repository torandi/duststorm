#ifndef SOUND_HPP
#define SOUND_HPP

#include <fmodex/fmod.hpp>

/**
 * A music player
 */
class Sound {
	public:
		/*
		 * Create a music player for the selected file.
		 */
		Sound(const char * file, int loops=0);
		Sound(const Sound &sound, int loops=0);
		~Sound();
	
		void set_delay(float t);

		void update(float t);

		bool is_playing() const;

		bool is_done() const;
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

		static void update_system();
	private:
		static FMOD::System * system_;
		static unsigned int system_usage_; //usage count

		static void initialize_fmod();
		static void terminate_fmod();

		static void errcheck(const char * contex);
	
		float delay;

		static FMOD_RESULT result_;

		Data * source;
		FMOD::Sound * sound_;
		int * sound_usage_count_;
		FMOD::Channel * channel_;

		double start_time;
};
#endif
