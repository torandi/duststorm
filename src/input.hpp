#ifndef INPUT_HPP
#define INPUT_HPP

#include <SDL/SDL.h>
#include <glm/glm.hpp>
#include "movable_object.hpp"

class Input {
	public:
	
		Input();
		~Input();

		static float movement_speed;
		static float rotation_speed;

		void parse_event(const SDL_Event &event);
		
		enum input_action_t {
			MOVE_X,
			MOVE_Y,
			MOVE_Z,
			ROTATE_X,
			ROTATE_Y,
			ROTATE_Z,
			ACTION_0,
			ACTION_1,
			ACTION_2,
			ACTION_3,
			START,
			NUM_ACTIONS
		};

		float current_value(input_action_t action) const;
		
		void update_object(MovableObject &obj, float dt) const;

		glm::vec3 movement_change() const;

		bool button_down(int btn);

		bool has_changed(input_action_t action, float epsilon) const;

		float normalized_trigger_value(int axis);
		float normalized_axis_value(int axis);
		float get_hat_up_down(int hat);
		float get_hat_right_left(int hat);
	private:
		//Sustained are caused by "key down" etc
		float sustained_values[NUM_ACTIONS];
		//Temporary are caused by mouse move etc
		float temporary_values[NUM_ACTIONS];
		//Used for changed method
		mutable float previous_value[NUM_ACTIONS];

		bool * moved_triggers;

		SDL_Joystick * joy;
};

#endif
