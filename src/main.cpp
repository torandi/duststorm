#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "engine.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "rendertarget.hpp"
#include "utils.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "time.hpp"
#include "cl.hpp"
#include "texture.hpp"
#include "timetable.hpp"

#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unistd.h>
#include <getopt.h>
#include <map>

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;
Time global_time(per_frame);

static volatile bool running = true;
static const char* program_name;
static bool resolution_given = false;

//These are all test variables that should be moved into a scene later
static Shader::lights_data_t lights;
static Light * light;
static int frames = 0;
static RenderTarget* composition;
static RenderTarget* downsample[3];
static glm::mat4 screen_ortho;           /* orthographic projection for primary fbo */
static XYLerpTable* particle_pos = nullptr;
static XYLerpTable* tv_pos = nullptr;
static XYLerpTable* test_pos = nullptr;
static std::map<std::string, Scene*> scene;

static void handle_sigint(int signum){
	if ( !running ){
		fprintf(stderr, "\rgot SIGINT again, aborting\n");
		abort();
	}

	running = false;
	fprintf(stderr, "\rgot SIGINT, terminating graceful\n");
}

static void show_fps(int signum){
	fprintf(stderr, "FPS: %d\n", frames);
	frames = 0;
}

namespace Engine {
	RenderTarget* rendertarget_by_name(const std::string& name){
		return nullptr;
	}
}

static void init(bool fullscreen){
	if ( SDL_Init(SDL_INIT_VIDEO) != 0 ){
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		exit(1);
	}

	const SDL_VideoInfo* vi = SDL_GetVideoInfo();
	if ( !vi ){ fprintf(stderr, "SDL_GetVideoInfo() failed\n"); abort(); }

	if ( fullscreen && !resolution_given ){
		resolution.x = vi->current_w;
		resolution.y = vi->current_h;
	}

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_SetVideoMode(resolution.x, resolution.y, 0, SDL_OPENGL|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
	SDL_EnableKeyRepeat(0, 0);
	SDL_WM_SetCaption("Speed 100%", NULL);

	if ( fullscreen ){
		SDL_ShowCursor(SDL_DISABLE);
	}

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	Engine::autoload_scenes();
	Engine::setup_opengl();
	Engine::load_shaders();

	lights.num_lights = 1;
	lights.ambient_intensity = glm::vec3(0.1, 0.1, 0.1);
	light = new Light(Light::POINT_LIGHT, glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.f, 0.f, 1.f));
	lights.lights[0] = light->shader_light();
	Shader::upload_lights(lights);

	screen_ortho = glm::ortho(0.0f, (float)resolution.x, 0.0f, (float)resolution.y, -1.0f, 1.0f);
	screen_ortho = glm::scale(screen_ortho, glm::vec3(1.0f, -1.0f, 1.0f));
	screen_ortho = glm::translate(screen_ortho, glm::vec3(0.0f, -(float)resolution.y, 0.0f));

	opencl = new CL();

	/* Preload common textures */
	fprintf(verbose, "Preloading textures\n");
	Texture2D::preload("default.jpg");

	/* Instantiate all scenes */
	scene["Test"]     = SceneFactory::create("Test",      glm::ivec2(resolution.x, resolution.y/3));
	scene["particle"] = SceneFactory::create("Particles", glm::ivec2(resolution.x/2, 2*resolution.y/3));
	scene["TV"]       = SceneFactory::create("TV",        glm::ivec2(resolution.x/2, 2*resolution.y/3));
	scene["Water"]    = SceneFactory::create("Water",     glm::ivec2(resolution.x/2, 2*resolution.y/3));

	/* Setup timetable */
	const char* tablename = PATH_SRC "timetable.txt";
	auto func = [](const std::string& name, float begin, float end){
		auto it = scene.find(name);
		if ( it != scene.end() ){
			it->second->add_time(begin, end);
		} else {
			fprintf(stderr, "Timetable entry for missing scene `%s', ignored.\n", name.c_str());
		}
	};
	if ( (ret=timetable_parse(tablename, func)) != 0 ){
		fprintf(stderr, "%s: failed to read `%s': %s\n", program_name, tablename, strerror(ret));
	}
	particle_pos = new XYLerpTable("scene/particles_pos.txt");
	tv_pos       = new XYLerpTable("scene/tv_pos.txt");
	test_pos     = new XYLerpTable("scene/test_pos.txt");

	composition   = new RenderTarget(resolution, false, false);
	downsample[0] = new RenderTarget(glm::ivec2(200, 200), false, false, GL_LINEAR);
	downsample[1] = new RenderTarget(glm::ivec2(100, 100), false, false, GL_LINEAR);
	downsample[2] = new RenderTarget(glm::ivec2( 50,  50), false, false, GL_LINEAR);

	global_time.set_paused(false); /* start time */
	checkForGLErrors("post init()");
}

static void cleanup(){
	for ( std::pair<std::string,Scene*> p : scene ){
		delete p.second;
	}
}

static void poll(){
	SDL_Event event;
	while ( SDL_PollEvent(&event) ){
		switch ( event.type ){
		case SDL_QUIT:
			running = false;
			break;

		case SDL_KEYDOWN:
			if ( event.key.keysym.sym == SDLK_ESCAPE ){
				running = false;
			}
			if ( event.key.keysym.sym == SDLK_q && event.key.keysym.mod & KMOD_CTRL ){
				running = false;
			}

			bool scale_updated = false;

			if ( event.key.keysym.sym == SDLK_SPACE ){
				global_time.toggle_pause();
				scale_updated = true;
			}

			if ( event.key.keysym.sym == SDLK_PERIOD ){
				global_time.step(1);
				scale_updated = true;
			}

			if ( event.key.keysym.sym == SDLK_COMMA ){
				global_time.adjust_speed(-10);
				scale_updated = true;
			} else if ( event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_p ){
				global_time.adjust_speed(10);
				scale_updated = true;
			}

			if ( scale_updated ){
				char title[64];
				sprintf(title, "Speed: %d%%", global_time.current_scale());
				SDL_WM_SetCaption(title, NULL);
			}
		}
	}
}

static void render_scene(){
	for ( std::pair<std::string,Scene*> p: scene ){
		p.second->render_scene();
	}
}

static void downsample_tv(){
	RenderTarget* prev = scene["TV"];
	for ( int i = 0; i < 3; i++ ){
		Shader::upload_state(downsample[i]->texture_size());
		Shader::upload_projection_view_matrices(downsample[i]->ortho(), glm::mat4());
		downsample[i]->with([prev,i](){
			prev->draw(shaders[SHADER_BLUR], glm::ivec2(0,0), downsample[i]->texture_size());
		});
		prev = downsample[i];
	}
}

static void render_composition(){
	RenderTarget::clear(Color::black);
	const float t = global_time.get();

	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(composition->ortho(), glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);

	/*glActiveTexture(GL_TEXTURE1);
	  texture_test->bind();
	  glActiveTexture(GL_TEXTURE0);

	  scene["TV"]->draw(shaders[SHADER_DISTORT], glm::ivec2(400,0));*/

	scene["Test"    ]->draw(shaders[SHADER_PASSTHRU], screen_pos(test_pos->at(t), glm::vec2(scene["Test"]->texture_size())));
	//scene["Water"   ]->draw(shaders[SHADER_PASSTHRU], screen_pos(tv_pos->at(t), glm::vec2(scene["Water"]->texture_size())));
	scene["TV"      ]->draw(shaders[SHADER_PASSTHRU], screen_pos(tv_pos->at(t), glm::vec2(scene["TV"]->texture_size())));
	scene["particle"]->draw(shaders[SHADER_PASSTHRU], screen_pos(particle_pos->at(t), glm::vec2(scene["particle"]->texture_size())));

	/*
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);*/
}

static void render_display(){
	RenderTarget::clear(Color::magenta);
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
	composition->draw(shaders[SHADER_FILMGRAIN]);
}

static void render(){
	checkForGLErrors("Frame begin");

	render_scene();
	downsample_tv();
	composition->with(render_composition);
	render_display();

	SDL_GL_SwapBuffers();
	checkForGLErrors("Frame end");
}

static void update(float dt){
	float t = global_time.get();
	for ( std::pair<std::string,Scene*> p: scene ){
		p.second->update_scene(t, dt);
	}
}

static void magic_stuff(){
	/* for calculating dt */
	struct timeval t, last;
	gettimeofday(&t, NULL);
	last = t;

	while ( running ){
		poll();

		/* calculate dt */
		struct timeval cur;
		gettimeofday(&cur, NULL);
		const uint64_t delta = (cur.tv_sec - t.tv_sec) * 1000000 + (cur.tv_usec - t.tv_usec);
		const  int64_t delay = per_frame - delta;

		global_time.update();
		update(global_time.dt());
		render();

		/* move time forward */
		frames++;
		last = cur;
		t.tv_usec += per_frame;
		if ( t.tv_usec > 1000000 ){
			t.tv_usec -= 1000000;
			t.tv_sec++;
		}

		/* fixed framerate */
		if ( delay > 0 ){
			usleep(delay);
		}
	}
}

void show_usage(){
	printf(PACKAGE_NAME "-" VERSION "\n"
	       "usage: %s [OPTIONS]\n"
	       "\n"
	       "  -r, --resolution=SIZE   Set window resultion (default: 800x600 in windowed and\n"
	       "                          current resolution in fullscreen.)\n"
	       "  -f, --fullscreen        Enable fullscreen mode (default: false)\n"
	       "  -w, --windowed          Inverse of --fullscreen.\n"
	       "  -v, --verbose           Enable verbose output\n"
	       "  -q, --quiet             Inverse of --verbose.\n"
	       "  -h, --help              This text\n",
	       program_name);
}

static int fullscreen = 0;
static int verbose_flag = 0;

static struct option options[] = {
	{"resolution",   required_argument, 0, 'r'},
	{"fullscreen",   no_argument,       &fullscreen, 1},
	{"windowed",     no_argument,       &fullscreen, 0},
	{"verbose",      no_argument,       &verbose_flag, 1},
	{"quiet",        no_argument,       &verbose_flag, 0},
	{"help",         no_argument,       0, 'h'},
	{0,0,0,0} /* sentinel */
};

int main(int argc, char* argv[]){
	/* extract program name from path. e.g. /path/to/MArCd -> MArCd */
	const char* separator = strrchr(argv[0], '/');
	if ( separator ){
		program_name = separator + 1;
	} else {
		program_name = argv[0];
	}

	/* parse arguments */
	int op, option_index;
	while ( (op = getopt_long(argc, argv, "r:fwvqh", options, &option_index)) != -1 ){
		switch ( op ){
		case 0:   /* long opt*/
		case '?': /* invalid */
			break;

		case 'r': /* --resolution */
		{
			int w,h;
			int n = sscanf(optarg, "%dx%d", &w, &h);
			if ( n != 2 || w <= 0 || h <= 0 ){
				fprintf(stderr, "%s: Malformed resolution `%s', must be WIDTHxHEIGHT. Option ignored\n", program_name, optarg);
			} else {
				resolution.x = w;
				resolution.y = h;
				resolution_given = true;
			}
		}
		break;

		case 'f': /* --fullscreen */
			fullscreen = 1;
			break;

		case 'w': /* --windowed */
			fullscreen = 0;
			break;

		case 'v': /* --verbose */
			verbose_flag = 1;
			break;

		case 'q': /* --quiet */
			verbose_flag = 0;
			break;

		case 'h': /* --help */
			show_usage();
			exit(0);

		default:
			fprintf(stderr, "%s: declared but unhandled argument '%c' (0x%02X)\n", program_name, op, op);
			abort();
		}
	};

	verbose = fopen(verbose_flag ? "/dev/stderr" : "/dev/null", "w");

	/* proper termination */
	signal(SIGINT, handle_sigint);

	/* setup FPS alarm handler */
	{
		struct itimerval difftime;
		difftime.it_interval.tv_sec = 1;
		difftime.it_interval.tv_usec = 0;
		difftime.it_value.tv_sec = 1;
		difftime.it_value.tv_usec = 0;
		signal(SIGALRM, show_fps);
		setitimer(ITIMER_REAL, &difftime, NULL);
	}

	/* let the magic begin */
	init(fullscreen);
	magic_stuff();
	cleanup();

	fclose(verbose);

	return 0;
}
