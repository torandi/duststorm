#include "input.hpp"
#include "movable_object.hpp"

#include <SDL/SDL.h>

#define AXIS_MAX 32767.f
#define DEAD_ZONE 0.2f

float Input::movement_speed = 4.f;
float Input::rotation_speed = 1.f;

Input::Input(){
	SDL_JoystickEventState(SDL_TRUE);
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	for(int i=0; i<NUM_ACTIONS; ++i) {
		sustained_values[i] = 0.0;
		temporary_values[i] = 0.0;
		previous_value[i] = -2.0;
	}
	if(SDL_NumJoysticks()>0){
		joy=SDL_JoystickOpen(0);
		moved_triggers = new bool[SDL_JoystickNumAxes(joy)];
		for(int i=0; i < SDL_JoystickNumAxes(joy); ++i)
			moved_triggers[i]=false;
	}
}

Input::~Input() {
	if(SDL_JoystickOpened(0) && joy)
		SDL_JoystickClose(joy);
}

void Input::parse_event(const SDL_Event &event) {
	memset(temporary_values, 0, NUM_ACTIONS * sizeof(float));
	switch(event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
				case SDLK_UP:
				case SDLK_w:
					//UP
					sustained_values[MOVE_Z] = movement_speed;
					break;
				case SDLK_DOWN:
				case SDLK_s:
					//DOWN
					sustained_values[MOVE_Z] = -movement_speed;
					break;
				case SDLK_LEFT:
				case SDLK_a:
					//LEFT
					sustained_values[MOVE_X] = -movement_speed;
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					//RIGHT
					sustained_values[MOVE_X] = movement_speed;
					break;
				default:
					break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
				case SDLK_UP:
				case SDLK_w:
					//UP
					sustained_values[MOVE_Z] = 0.f;
					break;
				case SDLK_DOWN:
				case SDLK_s:
					//DOWN
					sustained_values[MOVE_Z] = 0.f;
					break;
				case SDLK_LEFT:
				case SDLK_a:
					//LEFT
					sustained_values[MOVE_X] = 0.f;
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					//RIGHT
					sustained_values[MOVE_X] = 0.f;
					break;
				default:
					break;
			}
			break;
		case SDL_JOYAXISMOTION:
			moved_triggers[event.jaxis.axis]=true;
			break;
		case SDL_JOYBUTTONDOWN:
			switch(event.jbutton.button) {
				case 0:
					sustained_values[ACTION_0] = 1.f;
					break;
				case 1:
					sustained_values[ACTION_1] = 1.f;
					break;
				case 2:
					sustained_values[ACTION_2] = 1.f;
					break;
				case 3:
					sustained_values[ACTION_3] = 1.f;
					break;
			}
			break;
		case SDL_JOYBUTTONUP:
			switch(event.jbutton.button) {
				case 0:
					sustained_values[ACTION_0] = 0.f;
					break;
				case 1:
					sustained_values[ACTION_1] = 0.f;
					break;
				case 2:
					sustained_values[ACTION_2] = 0.f;
					break;
				case 3:
					sustained_values[ACTION_3] = 0.f;
					break;
			}
			break;
	}

	if(SDL_JoystickOpen(0)) {
		temporary_values[MOVE_X] = -normalized_axis_value(0)*movement_speed;
		temporary_values[MOVE_Y] = get_hat_up_down(0)*movement_speed;
		temporary_values[MOVE_Z] = -normalized_axis_value(1)*movement_speed;
		temporary_values[ROTATE_X] = -normalized_axis_value(4)*rotation_speed;
		temporary_values[ROTATE_Y] = -(normalized_trigger_value(5) - normalized_trigger_value(2))*2.f*rotation_speed;
		temporary_values[ROTATE_Z] = normalized_axis_value(3)*rotation_speed;
	}
}

glm::vec3 Input::movement_change() const {
	return glm::vec3(current_value(MOVE_X), current_value(MOVE_Y), current_value(MOVE_Z));
}

float Input::current_value(input_action_t action) const {
	return sustained_values[action] + temporary_values[action];
}

void Input::update_object(MovableObject &obj, float dt) const {
	obj.relative_move(movement_change()*dt);
	obj.pitch(current_value(ROTATE_X)*dt);
	obj.yaw(current_value(ROTATE_Y)*dt);
	obj.roll(current_value(ROTATE_Z)*dt);
}

bool Input::button_down(int btn) {
	return SDL_JoystickGetButton(joy, btn)==1;
}

float Input::normalized_axis_value(int axis) {
	int value = SDL_JoystickGetAxis(joy, axis);
	float n_value = value/AXIS_MAX;

	if(fabs(n_value) < DEAD_ZONE)
		return 0;
	else
		return n_value;
}

//For triggers
float Input::normalized_trigger_value(int axis) {
	if(moved_triggers[axis]) {
		int value = SDL_JoystickGetAxis(joy, axis);
		float n_value = (value+AXIS_MAX)/(2*AXIS_MAX);
		if(fabs(n_value) < DEAD_ZONE)
			return 0;
		else
			return n_value;
	} else {
		return 0;
	}
}

float Input::get_hat_up_down(int hat) {
	int val = SDL_JoystickGetHat(joy, hat);
	if(val & SDL_HAT_UP)
		return 1.f;
	else if(val & SDL_HAT_DOWN)
		return -1.f;
	else
		return 0.f;
	
}

float Input::get_hat_right_left(int hat) {
	int val = SDL_JoystickGetHat(joy, hat);
	if(val & SDL_HAT_RIGHT)
		return 1.f;
	else if(val & SDL_HAT_LEFT)
		return -1.f;
	else
		return 0.f;
}

bool Input::has_changed(Input::input_action_t action, float epsilon) const {
	float cur = current_value(action);
	bool changed = false;
	if(cur < (previous_value[action] - epsilon) || cur > (previous_value[action] + epsilon)) changed = true;

	previous_value[action] = cur;
	return changed;
}
