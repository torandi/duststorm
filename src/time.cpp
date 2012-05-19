#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "time.hpp"

#define USDIVIDER 1000000

Time::Time(int delta)
	: current({0,0})
	, prev(0.0f)
	, delta(delta)
	, scale(100)
	, steps(0)
	, paused(false) {

}

void Time::update(){
	/* single-stepping */
	if ( steps != 0 ){
		const long int usec = steps * delta;
		steps = 0;
		move(usec);
		return;
	}

	/* normal flow */
	const float k = (float)scale / 100.0f;
	const long int usec = delta * k;
	move(usec);
}

void Time::step(int amount){
	paused = true;
	scale = 0;
	steps += amount;
}

void Time::adjust_speed(int amount){
	scale += amount;
	paused = false;
}

int Time::current_scale() const {
	return scale;
}

void Time::toggle_pause(){
	scale = paused ? 100 : 0;
	paused = !paused;
}

float Time::get() const {
	return (float)current.tv_sec + (float)current.tv_usec / USDIVIDER;
}

const struct timeval& Time::timeval() const {
	return current;
}

float Time::dt() const {
	return prev;
}

void Time::move(long int usec){
	prev = (float)usec / USDIVIDER;
	if ( usec > 0 ){ /* forward */
		current.tv_usec += usec;
		while ( current.tv_usec > USDIVIDER ){
			current.tv_usec -= USDIVIDER;
			current.tv_sec++;
		}
	} else { /* backward */
		while ( usec > current.tv_usec ){
			current.tv_usec += USDIVIDER;
			current.tv_sec--;
		}
		current.tv_usec += usec;
	}
}
