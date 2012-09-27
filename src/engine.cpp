#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "engine.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "utils.hpp"

#include "sound.hpp"
#include "game.hpp" 
#include "config.hpp"

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

static void render_loading_scene() {
	checkForGLErrors("Frame begin");
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);

	Quad loadingscreen;
	loadingscreen.set_scale(glm::core::type::vec3(resolution.x,resolution.y,0));
	Texture2D* loadingtexture = Texture2D::from_filename(PATH_BASE "/data/textures/loading.png");
	loadingtexture->texture_bind(Shader::TEXTURE_2D_0);


	shaders[SHADER_PASSTHRU]->bind();

	loadingscreen.render();
	SDL_GL_SwapBuffers();
	checkForGLErrors("Frame end");
}

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
#ifdef WIN32
		if (useWII) {
			WII = new wii();
			WII->open();
			WII->connect();
		}
#endif
		render_loading_scene();

		Shader::fog_t fog = { glm::vec4(0.584f, 0.698f, 0.698f, 1.f), 0.005f };
		Shader::upload_fog(fog);
		srand(util_utime());
		opencl = new CL();

		render_loading_scene();

		Config config = Config::parse(PATH_BASE "/data/graphics.cfg");

		MovableLight::shadowmap_resolution = glm::ivec2(config["/shadowmap/resolution"]->as_vec2());
		MovableLight::shadowmap_far_factor = config["/shadowmap/far_factor"]->as_float();

		Game::init();
		render_loading_scene();
		game = new Game(level, config["/camera/near"]->as_float(), config["camera/far"]->as_float(), config["camera/fov"]->as_float());
	}

	void cleanup() {
		delete game;
#ifdef WIN32
		if (WII != nullptr) {
			delete WII;
			WII = nullptr;
		}
#endif
		Texture2D::cleanup();
	}

	void update(float dt) {
		Sound::update_system();
#ifdef WIN32
		if(useWII)
			WII->update();
#endif
		game->update(dt);	
	}

	void render() {
		game->render();
	}
}
