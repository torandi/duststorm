#ifndef TIME_HPP
#define TIME_HPP

#include <time.h>
#include <sys/time.h>

class Time {
public:
	/**
	 * @param delta How many µsec to move when running at 100% speed.
	 */
	Time(int delta);

	/**
	 * Move time forward (or backwards if speed is negative)
	 */
	void update();

	/**
	 * Single step time.
	 * @param amount How many steps to move forward or backward.
	 */
	void step(int amount);

	/**
	 * Change the time speed.
	 * @param amount By how much to adjust.
	 */
	void adjust_speed(int amount);

	/**
	 * Get current timescale.
	 */
	int current_scale() const;

	/**
	 * Toggle pause.
	 */
	void toggle_pause();

	/**
	 * Get current time as seconds in floating-point.
	 */
	float get() const;

	/**
	 * Set current time in number of steps.
	 */
	void set(unsigned long steps);

	/**
	 * Reset time to beginning.
	 * Same as set(0)
	 */
	void reset();

	/**
	 * Get current time as timeval.
	 */
	const struct timeval& timeval() const;

	/**
	 * Get the previous (scaled) delta.
	 */
	float dt() const;

private:
	void move(long int usec);

	struct timeval current;
	float prev;
	int delta;
	int scale;
	int steps;
	bool paused;
};

#endif /* TIME_HPP */
