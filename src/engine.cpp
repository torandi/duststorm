#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "engine.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "utils.hpp"

#include "sound.hpp"
#include "game.hpp" 
#include "wii.hpp"

CL * opencl;

static const char* shader_programs[NUM_SHADERS] = {
	"simple",
	"normal",
	"debug",
	"skybox",
	"water",
	"particles",
	"passthru",
	"blur",
	"blend"
};

namespace Engine {
	Game * game;

	void setup_opengl(){
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_BLEND);
		glCullFace(GL_BACK);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}

	void load_shaders() {
		for(int i=0; i < NUM_SHADERS; ++i) {
			shaders[i] = Shader::create_shader(shader_programs[i]);
		}
	}

	void input(const SDL_Event &event) {
		game->handle_input(event);
	}

	void init(const std::string &level) {
		if (useWII) {
			WII = new wii();
			WII->open();
			WII->connect();
		}
		Shader::fog_t fog = { glm::vec4(0.584f, 0.698f, 0.698f, 1.f), 0.005f };
		Shader::upload_fog(fog);
		srand(util_utime());
		opencl = new CL();
		Game::init();
		game = new Game(level);
	}

	void cleanup() {
		delete game;
		if (WII != nullptr) {
			delete WII;
			WII = nullptr;
		}
		Texture2D::cleanup();
	}

	void update(float dt) {
		Sound::update_system();
		WII->update();
		game->update(dt);	
	}

	void render() {
		game->render();
	}
}
