#ifndef WII_HPP
#define WII_HPP

// Library.
#include <wiimote.h>

// Wrapper class around library.
class wii
{
public:

	wiimote* wiim;

	wii();
	~wii();

	// Initialize the wiimote; do this at application startup.
	void open();
	// Connect (or reconnect, if connection was lost).
	void connect();
	// Whether the wiimote is connected.
	bool connected() const;
	// Whether the motion plus addon is active.
	bool motionPlus() const;
	// Update state. Do this once per frame.
	void update();
	// Close the wiimote.
	void close();
	// Getters.
	const float& getPitch() const { return pitch; }
	const float& getRoll() const { return roll; }
	const bool& getButtonADown() const { return downA; }
	const bool& getButtonAPressed() const { return pressedA; }
	const bool& getButtonBDown() const { return downB; }
	const bool& getButtonBPressed() const { return pressedB; }
	const bool& getButtonPlusDown() const { return downPlus; }
	const bool& getButtonPlusPressed() const { return pressedPlus; }
	const float& getYaw() const { return yaw; }
	// Setters
	void setRumble(const bool& rumble);

private:

	// Cached pitch and roll.
	float pitch, roll;
	// State for 'A' button.
	bool downA, pressedA;
	// State for 'B' button.
	bool downB, pressedB;
	// State for '+' button.
	bool downPlus, pressedPlus;
	// Yaw estimation values.
	float yaw;
	float yawCal;
	float yawThreshold;

	// Initialize fields.
	void init();
	// Library helper method.
	static void on_state_change(wiimote& remote,
		state_change_flags  changed,
		const wiimote_state &new_state);
};

#endif
