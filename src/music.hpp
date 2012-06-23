#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <portaudio.h>
#include <iostream>
#include <vorbis/vorbisfile.h>
#include <pthread.h>
#include <pulse/pulseaudio.h>

#define DEFAULT_BUFFER_SIZE 1024*512
#define OGG_BUFFER_SIZE 4096

//Time to sleep when hitting a full sound buffer
#define OVERFILL_SLEEP 500 

#define ENDIAN 0

/**
 * A music player
 */
class Music {
	public:
		/*
		 * Create a music player for the selected file.
		 * Optional: Specify buffer size in bytes
		 */
		Music(const char * file, int buffer_size_ = DEFAULT_BUFFER_SIZE);
		~Music();

		/**
		 * Start playing the file.
		 * @param num_loops: Number of times to loop the file, -1 loops forever
		 */
		void play(int num_loops = 0);
		/*
		 * Stop the playback
		 */
		void stop();
		/**
		 * Returns the time since playback started
		 */
		double time() const;

		/*
		 * This function will be called when the music finishes to play
		 * Optional argument data is passed to the function
		 */
		void set_finished_callback(void (*callback)(void*), void * data = NULL); 

	private:
		static int pa_contexts;
		static void initialize_pa();
		static void terminate_pa();
		static void pulse_context_callback(pa_context * c, void * userdata);
		static void pulse_server_info_callback (pa_context *c, const pa_server_info *i, void *userdata);
		static void pulse_sink_info_callback (pa_context *c, const pa_sink_info *i, int eol, void *userdata);

		//Find hw device to use
		static void find_default_device();
		static short hw_device[2];

		static int device_index;
		static PaTime device_latency;
		static pa_mainloop * pulse_main;

		FILE * source;
		OggVorbis_File ogg_file;
		PaStream * stream;
		double sample_rate;
		int num_channels;
		pthread_t decoder_thread;
		int buffer_size;
		char * ogg_buffer;
		int16_t * buffer;
		int16_t * buffer_write;
		int16_t * buffer_read;
	
		double start_time;
		int loops_remaining;
		bool playing;
		bool decode;

		void (*finished_callback)(void*);
		void *callback_data;

		void load_ogg(const char * filename);
		bool buffer_data();
		void run_decode();
		static void *decode_thread_helper(void * data);
		void start_decode();
		void stop_decode();
		void reset_ogg_position();
		bool eof_reached;

		int16_t * next_ptr(int16_t *ptr) const;

		static int pa_callback(const void *inputBuffer,
										void *outputBuffer,
										unsigned long framesPerBuffer,
										const PaStreamCallbackTimeInfo* timeInfo,
										PaStreamCallbackFlags statusFlags,
										void *userData );

		static void pa_finished(void *userData);

		static void print_pa_error(const char * context, const PaError &err);

		static PaStreamParameters output_params;
};
#endif
