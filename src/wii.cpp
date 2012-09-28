#include "wii.hpp"
#include <wiimote.h>
#include <cassert>
#include <cmath>
using namespace std;

wii::wii()
{
	init();
}

wii::~wii()
{
	close();
}

void wii::init()
{
	pitch = roll = 0;
	downA = pressedA = downB = pressedB = false;
	yaw = 0;
	yawCal = 0;
	yawThreshold = 10;
}

void wii::open()
{
	wiim = new wiimote();
	// use a state-change callback to get notified of
	//  extension-related events, and polling for everything else
	wiim->ChangedCallback		= wii::on_state_change;
	//  notify us only when the wiimote connected sucessfully, or something
	//   related to extensions changes
	wiim->CallbackTriggerFlags = (state_change_flags)(CONNECTED | EXTENSION_CHANGED | MOTIONPLUS_CHANGED);
}

void wii::connect()
{
	assert(wiim != nullptr);
	fprintf(stderr, "Wiimote: Connecting...");
	while (!wiim->Connect(wiimote::FIRST_AVAILABLE)) {
		fprintf(stderr, ".");
		Sleep(1000);
	}
	fprintf(stderr, "\n");
	wiim->SetLEDs(0x0f); // Turn on ALL the lights!
	fprintf(stderr, "Wiimote: Connected!\n");
}

bool wii::connected() const
{
	return wiim != nullptr && wiim->IsConnected();
}

bool wii::motionPlus() const
{
	return wiim != nullptr && wiim->ExtensionType == wiimote_state::MOTION_PLUS;
}

void wii::update()
{
	assert(wiim != nullptr);
	// IMPORTANT: the wiimote state needs to be refreshed each pass
	if (wiim->RefreshState() == NO_CHANGE) return;

	float newPitch = wiim->Acceleration.Orientation.Pitch;
	if (newPitch > -90 && newPitch < 90) pitch = newPitch; // Ignore values outside range.

	roll = wiim->Acceleration.Orientation.Roll;

	bool tmp = wiim->Button.A();
	pressedA = tmp && !downA;
	downA = tmp;

	tmp = wiim->Button.B();
	pressedB = tmp && !downB;
	downB = tmp;

	tmp = wiim->Button.Plus();
	pressedPlus = tmp && !downPlus;
	downPlus = tmp;

	//pressedArrowDown = wiim->Button.Down();
	tmp = wiim->Button.Down();
	pressedDown = tmp && !downDown;
	downDown = tmp;

	tmp = wiim->Button.Up();
	pressedUp = tmp && !downUp;
	downUp = tmp;
	
	tmp = wiim->Button.Left();
	pressedLeft = tmp && !downLeft;
	downUp = tmp;

	tmp = wiim->Button.Right();
	pressedRight = tmp && !downRight;
	downRight = tmp;
}

void wii::close()
{
	if (wiim == nullptr) return;
	wiim->SetLEDs(0x00); // Clear lights.
	Sleep(500);
	delete wiim;
	wiim = nullptr;
}

void wii::setRumble(const bool& rumble)
{
	if (wiim == nullptr) return;
	wiim->SetRumble(rumble);
}

void wii::on_state_change (wiimote& remote,
						   state_change_flags  changed,
						   const wiimote_state &new_state)
{
	// we use this callback to set report types etc. to respond to key events
	//  (like the wiimote connecting or extensions (dis)connecting).

	// NOTE: don't access the public state from the 'remote' object here, as it will
	//		  be out-of-date (it's only updated via RefreshState() calls, and these
	//		  are reserved for the main application so it can be sure the values
	//		  stay consistent between calls).  Instead query 'new_state' only.

	// the wiimote just connected
	if(changed & CONNECTED)
	{
		// ask the wiimote to report everything (using the 'non-continous updates'
		//  default mode - updates will be frequent anyway due to the acceleration/IR
		//  values changing):

		// note1: you don't need to set a report type for Balance Boards - the
		//		   library does it automatically.

		// note2: for wiimotes, the report mode that includes the extension data
		//		   unfortunately only reports the 'BASIC' IR info (ie. no dot sizes),
		//		   so let's choose the best mode based on the extension status:
		if(new_state.ExtensionType != wiimote::BALANCE_BOARD)
		{
			if(new_state.bExtension)
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT); // no IR dots
			else
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);		//    IR dots
		}
	}
	// a MotionPlus was detected
	if(changed & MOTIONPLUS_DETECTED)
	{
		// enable it if there isn't a normal extension plugged into it
		// (MotionPlus devices don't report like normal extensions until
		//  enabled - and then, other extensions attached to it will no longer be
		//  reported (so disable the M+ when you want to access them again).
		if(remote.ExtensionType == wiimote_state::NONE) {
			bool res = remote.EnableMotionPlus();
			_ASSERT(res);
		}
	}
	// an extension is connected to the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_CONNECTED)
	{
		// We can't read it if the MotionPlus is currently enabled, so disable it:
		if(remote.MotionPlusEnabled())
			remote.DisableMotionPlus();
	}
	// an extension disconnected from the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_DISCONNECTED)
	{
		// enable the MotionPlus data again:
		if(remote.MotionPlusConnected())
			remote.EnableMotionPlus();
	}
	// another extension was just connected:
	else if(changed & EXTENSION_CONNECTED)
	{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(1000, 200);
#endif
		// switch to a report mode that includes the extension data (we will
		//  loose the IR dot sizes)
		// note: there is no need to set report types for a Balance Board.
		if(!remote.IsBalanceBoard())
			remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
	}
	// extension was just disconnected:
	else if(changed & EXTENSION_DISCONNECTED)
	{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(200, 300);
#endif
		// use a non-extension report mode (this gives us back the IR dot sizes)
		remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);
	}
}
