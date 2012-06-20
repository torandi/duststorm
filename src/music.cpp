#include "music.hpp"
#include "globals.hpp"

#include <portaudio.h>
#include <iostream>
#include <vorbis/vorbisfile.h>

int Music::pa_contexts = 0;

void Music::initialize_pa() {
	PaError err = Pa_Initialize();
	print_pa_error("initialize", err);
}

void Music::terminate_pa() {
	PaError err = Pa_Terminate();
	print_pa_error("terminate", err);
}

void Music::print_pa_error(const char * context, const PaError &err) {
	if(err == paNoError) return;
	printf("[Music] Error in %s: %s\n", context, Pa_GetErrorText( err ));
	abort();
}

void Music::set_finished_callback(void (*callback)(void*), void * data) {
	finished_callback = callback;
	callback_data = data;
}


void Music::pa_finished(void *userData) {
	Music * m = (Music*) userData;
	//The stream might just be inactive (due to eof)
	if( ! Pa_IsStreamStopped ( m->stream ) ) Pa_StopStream( m->stream );

	m->stop_decode();

	m->playing = false;
	if(m->finished_callback != NULL) (*(m->finished_callback)) ( m->callback_data );

	printf("[Music] Finish callback called\n");

}

int Music::pa_callback(const void *inputBuffer,
													void *outputBuffer,
													unsigned long framesCount,
													const PaStreamCallbackTimeInfo* timeInfo,
													PaStreamCallbackFlags statusFlags,
													void *userData ) {
	Music * m = (Music*) userData;
	int16_t * next;
	int16_t * out = (int16_t*) outputBuffer;
	for(unsigned long i = 0; i < framesCount*m->num_channels; ++i) {
		next = m->next_ptr(m->buffer_read);
		if(next == m->buffer_write) {
			//Fill the rest with silence:
			for(;i < framesCount*m->num_channels; ++i) {
				*out++ = 0;
			}

			if(m->eof_reached) {
				return paComplete; //We have played all data
			} else {
				fprintf(stderr, "[Music] Critical error: No data left in buffer\n");
				return paAbort;
			}
		} else {
			*out++ = *(m->buffer_read);
			++(m->buffer_read);
		}
	}
	return paContinue;
}

Music::Music(const char * file, int buffer_size_) :
		buffer_size(buffer_size_)
	, playing(false) 
	, decode(false) 
	, finished_callback(nullptr)
	, callback_data(nullptr) {
	if(pa_contexts == 0) initialize_pa();
	++pa_contexts;

	buffer = (int16_t*) malloc(sizeof(int16_t)*buffer_size);
	ogg_buffer = (char*) malloc(sizeof(char)*OGG_BUFFER_SIZE);

	load_ogg(file);

	PaError err = Pa_OpenDefaultStream(&stream,
			0,
			num_channels,
			paInt16,
			sample_rate,
			paFramesPerBufferUnspecified,
			&Music::pa_callback,
			this);
	print_pa_error("create stream", err);
	err = Pa_SetStreamFinishedCallback(stream, &Music::pa_finished);
}

Music::~Music() {
	PaError err = Pa_CloseStream( stream );
	print_pa_error("close stream", err);

	free(buffer);
	free(ogg_buffer);

	ov_clear(&ogg_file);
	fclose(source);

	--pa_contexts;
	if(pa_contexts == 0) terminate_pa();
}

void Music::play(int num_loops) {
	if(playing) {
		printf("[Music] Warning, called Music::play() on playing stream\n");
		return;
	}
	loops_remaining = num_loops;

	reset_ogg_position();
	buffer_write = buffer;
	buffer_read = buffer + buffer_size - 1;

	eof_reached = false;
	playing = true;
	start_decode();
	Pa_StartStream( stream );
}

void Music::stop() {
	PaError err = Pa_StopStream( stream );
	print_pa_error("stop stream", err);
}

void Music::reset_ogg_position() {
	int seek=ov_time_seek(&ogg_file,0);
	if(seek!=0) {
		switch(seek) {
			case OV_ENOSEEK:
					printf("Bitstream is not seekable.\n");
					break;
			case OV_EINVAL:
					printf("Invalid argument value; possibly called with an OggVorbis_File structure that isn't open.\n");
					break;
			case OV_EREAD:
					printf("A read from media returned an error.\n");
					break;
			case OV_EFAULT:
					printf("Internal logic fault; indicates a bug or heap/stack corruption.\n");
					break;
			case OV_EBADLINK:
					printf("Invalid stream section supplied to libvorbisfile, or the requested link is corrupt.");
					break;
		}
		abort();
	}
}

int16_t * Music::next_ptr(int16_t *ptr) const{
	int16_t * next = ptr+1;
	if(next >= (buffer+buffer_size) )
		return buffer;
	else
		return next;
}

void Music::load_ogg(const char * filename) {
	vorbis_info *pInfo;

	source = fopen(filename,"r");

	int status=ov_open(source,&ogg_file,NULL,0);

	if(status!=0) {
		switch(status) {
			case OV_EREAD:
				printf("[Music] Failed to read %s\n",filename);
				break;
			case OV_ENOTVORBIS:
				printf("[Music] %s is not a vorbis file.\n",filename);
				break;
			case OV_EVERSION:
				printf("[Music] %s have wrong vorbis version.\n",filename);
				break;
			case OV_EBADHEADER:
				printf("[Music] %s have invalid vorbis header.\n",filename);
				break;
			case OV_EFAULT:
				printf("[Music] Internal error when loading %s.\n",filename);
				break;
		}
		abort();
	}

	pInfo=ov_info(&ogg_file,-1);

	if(pInfo) {
		num_channels = pInfo->channels;

		sample_rate = (double) pInfo->rate;

		fprintf(verbose,"[Music] %s loaded. Channels: %d Freq: %f Hz \n",filename,num_channels, sample_rate);

	} else {
		fprintf(stderr,"[Music] Failed to get ov_info from file %s\n",filename);
	}
}

void Music::start_decode() {
	decode = true;
	pthread_create(&decoder_thread, NULL, &Music::decode_thread_helper, this);
}

void Music::stop_decode() {
	decode = false;
	pthread_join(decoder_thread, nullptr);
}

void * Music::decode_thread_helper(void * data) {
	((Music*) data)->run_decode();
	pthread_exit(NULL);
	return nullptr;
}

void Music::run_decode() {
	while(decode) {
		if(!buffer_data()) {
			if(loops_remaining > 0)
				reset_ogg_position();
			else
				eof_reached = true;
			--loops_remaining;
		}
	}
}

bool Music::buffer_data() {
	int bit_stream;
	//Decode data
	int bytes = ov_read(&ogg_file,ogg_buffer, OGG_BUFFER_SIZE, ENDIAN,2,1,&bit_stream);

	if(bytes < 0) {
		switch(bytes) {
			case OV_HOLE:
				fprintf(stderr, "[Music] Error in vorbis decode: data interruption (OV_HOLE)\n");
				abort();
			case OV_EBADLINK:
				fprintf(stderr, "[Music] Error in vorbis decode: invalid stream section (OV_EBADLINK)\n");
				abort();
			case OV_EINVAL:
				fprintf(stderr, "[Music] Error in vorbis decode: corrupt data (OV_EINVAL)\n");
				abort();
		}
		return false;
	} else if(bytes == 0) {
		return false;
	} else {
		//Move data to sound buffer:
		char * pos = ogg_buffer;
		while(decode && bytes > 0) {
			int16_t * next = next_ptr(buffer_write);
			while(decode && next == buffer_read) {
				printf("Buffer overfilled\n");
				usleep(OVERFILL_SLEEP);
			}
			if(!decode) return false;
			*buffer_write = *((int16_t*) pos); //Write
			buffer_write = next; //Advance pointer
			bytes -= sizeof(int16_t)/sizeof(char);
			pos += sizeof(int16_t)/sizeof(char);
		}
		return true;
	}
}
