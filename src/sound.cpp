#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "data.hpp"
#include "sound.hpp"
#include "globals.hpp"

#include <fmodex/fmod_errors.h>

#define MAX_CHANNELS 100

static const bool mute_sound = false;

FMOD::System * Sound::system_ = nullptr;
unsigned int Sound::system_usage_ = 0;
FMOD_RESULT Sound::result_;

void Sound::initialize_fmod() {
	result_ = FMOD::System_Create(&system_);
	errcheck("create system");
	result_ = system_->init(MAX_CHANNELS, FMOD_INIT_NORMAL, NULL);
	errcheck("initialize system");
}

void Sound::errcheck(const char * contex) {
	if(result_ != FMOD_OK) {
		printf("[Sound] FMOD error in %s %s(%d))\n", contex, FMOD_ErrorString(result_), result_);
		abort();
	}
}

void Sound::terminate_fmod() {
	result_ = system_->release();
	errcheck("terminate fmod");
}

Sound::Sound(const char * file, int loops) : delay(-0.1f) {
	if(system_usage_ == 0) initialize_fmod();
	++system_usage_;

	char* real_path = (char*) malloc(sizeof(PATH_BASE) + sizeof(file) + 32);
	if ( sprintf(real_path, "%s%s", PATH_BASE "/data/sfx/", file) == -1 ){
		abort();
	}
	source = Data::open(real_path);
	if(source == NULL) {
		printf("[Sound] Couldn't open file %s\n", real_path);
		abort();
	}

	free(real_path);

	FMOD_CREATESOUNDEXINFO info = {0, };
	info.cbsize = sizeof(info);
	info.length = source->size();

	result_ = system_->createSound((const char*) source->data(), FMOD_OPENMEMORY, &info, &sound_);
	errcheck("create sound");
	result_ = system_->playSound(FMOD_CHANNEL_FREE, sound_, true /* paused */, &channel_);
	errcheck("start sound (paused)");
	result_ = channel_->setLoopCount(loops);
	errcheck("set loops");
	if(mute_sound) {
		channel_->setMute(true);
		printf("WARNING! Sound is MUTED!\n");
	}

	sound_usage_count_ = new int;
	++(*sound_usage_count_);
}

Sound::Sound(const Sound &sound, int loops) : 
	delay(-0.1f)
	,	source(sound.source)
	, sound_(sound.sound_)
	, sound_usage_count_(sound.sound_usage_count_)
{
	++system_usage_;

	++(*sound_usage_count_);

	result_ = system_->playSound(FMOD_CHANNEL_FREE, sound_, true /* paused */, &channel_);
	if(result_ != FMOD_OK) {
		fprintf(verbose, "[FMOD] Failed to start playing sound\n");
		channel_ = nullptr;
		return;
	}
	result_ = channel_->setLoopCount(loops);
	errcheck("set loop count");
}

Sound::~Sound() {

	if(channel_ != nullptr) 
		channel_->stop();

	--(*sound_usage_count_);
	if(*sound_usage_count_ <= 0) {
		delete sound_usage_count_;
		delete source;
		result_ = sound_->release();
		errcheck("sound::release()");
	}

	--system_usage_;
	if(system_usage_ == 0) terminate_fmod();
}

void Sound::set_delay(float t) {
	delay = t;
}

void Sound::update_system() {
	system_->update();
}

void Sound::update(float dt) {
	if(channel_ == nullptr) return;
	if(delay > 0.f) {
		delay -= dt;
		if(delay <= 0.f)
			play();
	}
}

bool Sound::is_done() const {
	if(channel_ == nullptr) return false;
	if(delay > 0.f)
		return false;
	return !is_playing();
}

bool Sound::is_playing() const {
	if(channel_ == nullptr) return false;
	bool is_playing, is_paused = false;
	result_ = channel_->isPlaying(&is_playing);
	if(result_ != FMOD_OK) return false;
	if(is_playing) {
		result_ = channel_->getPaused(&is_paused);
		if(result_ != FMOD_OK) return false;
	}
	return is_playing && !is_paused;
}

void Sound::play() {
	if(channel_ == nullptr) return;
	result_ = channel_->setPosition(0, FMOD_TIMEUNIT_MS);
	errcheck("Sound::play() setPosition(0)");
	result_ = channel_->setPaused(false);
	errcheck("Sound::play() setPaused(false)");
}

void Sound::stop() {
	if(channel_ == nullptr) return;
	channel_->setPaused(true);
}

double Sound::time() const {
	if(channel_ == nullptr) return -1.0;
	unsigned int pos;
	result_ = channel_->getPosition(&pos, FMOD_TIMEUNIT_MS);
	errcheck("Sound::time()");
	return pos / 1000.0;
}

void Sound::seek(double t) {
	if(channel_ == nullptr) return;
	result_ = channel_->setPosition(t*1000.0, FMOD_TIMEUNIT_MS);
	errcheck("Sound::seek()");
}

float Sound::get_volume()
{
	if(channel_ == nullptr) return -1;
	float vol=0;
	channel_->getVolume(&vol);
	return vol;
}

void Sound::set_volume(float vol)
{
	if(channel_ == nullptr) return;
	channel_->setVolume(vol);
}