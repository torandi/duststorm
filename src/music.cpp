#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "music.hpp"
#include "globals.hpp"

#include <fmodex/fmod_errors.h>

#define MAX_CHANNELS 10

FMOD::System * Music::system_ = nullptr;
unsigned int Music::system_usage_ = 0;
FMOD_RESULT Music::result_;

void Music::initialize_fmod() {
	result_ = FMOD::System_Create(&system_);
	errcheck("create system");
	result_ = system_->init(MAX_CHANNELS, FMOD_INIT_NORMAL, NULL);
	errcheck("initialize system");
}

void Music::errcheck(const char * contex) {
	if(result_ != FMOD_OK) {
		printf("[Music] FMOD error in %s %s(%d))\n", contex, FMOD_ErrorString(result_), result_);
		abort();
	}
}

void Music::terminate_fmod() {
	result_ = system_->release();
	errcheck("terminate fmod");
}

Music::Music(const char * file) {
	if(system_usage_ == 0) initialize_fmod();
	++system_usage_;

	char* real_path;
	if ( asprintf(&real_path, "%s%s", PATH_BASE "/music/", file) == -1 ){
		abort();
	}
	source = Data::open(real_path);
	if(source == NULL) {
		printf("[Music] Couldn't open file %s\n", real_path);
		abort();
	}
	
	free(real_path);

	FMOD_CREATESOUNDEXINFO info = {0, };
	info.cbsize = sizeof(info);
	info.length = source->size();

	//result_ = system_->createSound((const char*) source->data(), FMOD_DEFAULT | FMOD_OPENMEMORY | FMOD_2D, &info, &sound_);
	result_ = system_->createSound((const char*) source->data(), FMOD_OPENMEMORY, &info, &sound_);
	errcheck("create sound");
	result_ = system_->playSound(FMOD_CHANNEL_FREE, sound_, true /* paused */, &channel_);
	errcheck("start sound (paused)");
}

Music::~Music() {
	result_ = sound_->release();
	errcheck("sound::release()");

	delete source;

	--system_usage_;
	if(system_usage_ == 0) terminate_fmod();
}

bool Music::is_playing() const {
	bool is_playing, is_paused = false;
	result_ = channel_->isPlaying(&is_playing);
	errcheck("Get channel::isPlaying()");
	if(is_playing) {
		result_ = channel_->getPaused(&is_paused);
		errcheck("Get channel::getPaused()");
	}
	return is_playing && !is_paused;
}

void Music::play() {
	if(is_playing()) {
		printf("[Music] Warning, called Music::play() on playing stream\n");
		return;
	}
	result_ = channel_->setPosition(0, FMOD_TIMEUNIT_MS);
	errcheck("Music::play() setPosition(0)");
	result_ = channel_->setPaused(false);
	errcheck("Music::play() setPaused(false)");
}

void Music::stop() {
	channel_->setPaused(true);
}

double Music::time() const {
	unsigned int pos;
	result_ = channel_->getPosition(&pos, FMOD_TIMEUNIT_MS);
	errcheck("Music::time()");
	return pos / 1000.0;
}

void Music::seek(double t) {
		result_ = channel_->setPosition(t*1000.0, FMOD_TIMEUNIT_MS);
		errcheck("Music::seek()");
}
