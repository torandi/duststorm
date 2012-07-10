#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>

class Config {
	public:
		enum input_action_t {
			MOVE_UP = 0,
			MOVE_DOWN,
			MOVE_LEFT,
			MOVE_RIGHT,
			ACTION_1,
			ACTION_2,
			ACTION_3,
			ACTION_4,
			ACTION_5,
			ACTION_6,
			ACTION_7,
			ACTION_8,
			ACTION_9,
			ACTION_10,
			NUM_INPUT_ACTIONS
		};

		input_action_t mapping(int key) const;

		void interactive_configure();
	
		void load();
		void save() const;

		int keymap[NUM_INPUT_ACTIONS];
	private:
		static const char * action_names[];
};

#endif
