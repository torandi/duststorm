#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "config.hpp"
#include "data.hpp"
#include "yaml-helper.hpp"

#include <SDL/SDL.h>
#include <fstream>

void Config::interactive_configure() {
	printf("Enter keyboard configuration\n");
	
	for(int i=0; i < NUM_INPUT_ACTIONS; ++i) {
		printf("Enter key for %s: ", action_names[i]);
		fflush(stdout);
		bool loop = true;
		SDL_Event event;
		while(loop) {
			if(SDL_PollEvent(&event) && event.type == SDL_KEYDOWN) {
				loop = false;
				keymap[i] = (int) event.key.keysym.sym;
				printf("%s\n", SDL_GetKeyName(event.key.keysym.sym));
			}
		}
	}
}

void Config::load() {
	YAML::Node config = YAML::LoadFile(PATH_BASE"/game/config.yaml");
	for(int i = 0; i < NUM_INPUT_ACTIONS; ++i) {
		keymap[i] = config["keys"][action_names[i]].as<int>();
	}
}

void Config::save() const {
	YAML::Node config;
	YAML::Node keys;
	for(int i = 0; i < NUM_INPUT_ACTIONS; ++i) {
		keys[action_names[i]] = keymap[i];
	}
	config["keys"] = keys;
	std::ofstream out(PATH_BASE"/game/config.yaml");
	out << config;
	out.close();
}

const char * Config::action_names[] = {
	"move_up",
	"move_down",
	"move_left",
	"move_right",
	"action_1",
	"action_2",
	"action_3",
	"action_4",
	"action_5",
	"action_6",
	"action_7",
	"action_8",
	"action_9",
	"action_10"
};
