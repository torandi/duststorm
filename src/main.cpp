#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#define GAME_NAME "Dust Storm"

#include "engine.hpp"
#include "globals.hpp"
#include "render_object.hpp"
#include "rendertarget.hpp"
#include "utils.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "quad.hpp"


#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifndef WIN32
	#include <unistd.h>
	#include <getopt.h>
	#include <sys/time.h>
#endif

#include <map>
#include <ctime>

#define LOGFILE PATH_BASE "frob.log"

#define GAME_VERSION "0.3"

glm::mat4 screen_ortho;           /* orthographic projection for primary fbo */

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;
float global_time = 0.f;
static volatile bool running = true;
static const char* program_name;
static bool resolution_given = false;
static int frames = 0;

static std::string level = "default";

static void poll();

namespace Engine {
	void terminate() {
		::running = false;
	}
}

static void handle_sigint(int signum){
	if ( !running ){
		fprintf(stderr, "\rgot SIGINT again, util_aborting\n");
		abort();
	}

	running = false;
	fprintf(stderr, "\rgot SIGINT, terminating graceful\n");
}

static void show_fps(int signum){
	fprintf(stderr, "FPS: %d\n", frames);
	frames = 0;
}

static void init(bool fullscreen, bool vsync){
	if ( SDL_Init(SDL_INIT_VIDEO) != 0 ){
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		exit(1);
	}

	const SDL_VideoInfo* vi = SDL_GetVideoInfo();
	if ( !vi ){ fprintf(stderr, "SDL_GetVideoInfo() failed\n"); util_abort(); }

	if ( fullscreen && !resolution_given ){
		resolution.x = vi->current_w;
		resolution.y = vi->current_h;
	}

	if(vsync) SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_SetVideoMode(resolution.x, resolution.y, 0, SDL_OPENGL|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
	SDL_EnableKeyRepeat(0, 0);
	SDL_WM_SetCaption(GAME_NAME " " GAME_VERSION, NULL);

	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	fprintf(verbose,"OpenGL Device: %s - %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));

	Engine::setup_opengl();

	Shader::initialize();
	Engine::load_shaders();

	screen_ortho = glm::ortho(0.0f, (float)resolution.x, 0.0f, (float)resolution.y, -1.0f, 1.0f);
	screen_ortho = glm::scale(screen_ortho, glm::vec3(1.0f, -1.0f, 1.0f));
	screen_ortho = glm::translate(screen_ortho, glm::vec3(0.0f, -(float)resolution.y, 0.0f));

	GLint max_texture_units;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);
	fprintf(verbose, "Supports %d texture units\n", max_texture_units);

	Engine::init(level);


	checkForGLErrors("post init()");

}

static void cleanup(){
	Engine::cleanup();
	Shader::cleanup();
	SDL_Quit();
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
		}
		Engine::input(event);
	}
}

static void render(){
	checkForGLErrors("Frame begin");
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	Engine::render();

	SDL_GL_SwapBuffers();
	checkForGLErrors("Frame end");
}

static void update(float dt){
	global_time += dt;
	Engine::update(dt);
}

static void main_loop(){
	/* for calculating dt */
	long t, last;
	t = util_utime();
	last = t;
	
	while ( running ){
		
		poll();
		

		long cur = util_utime();
		const long delta = cur - t;
		const long delay = per_frame - delta;


		float dt = (cur - last)/1000000.0 ;

		update(dt);
		render();

		/* move time forward */
		frames++;
		last = cur;
		t += per_frame;


		/* fixed framerate */
		if ( delay > 0 ){
			util_usleep(delay);
		}

	}
}

void show_usage(){
#ifndef WIN32
	printf(GAME_NAME " " GAME_VERSION " (based on fubar engine)\n"
	       "usage: %s [OPTIONS]\n"
	       "\n"
	       "  -r, --resolution=SIZE   Set window resultion (default: 800x600 in windowed and\n"
	       "                          current resolution in fullscreen.)\n"
	       "  -f, --fullscreen        Enable fullscreen mode (default: %s)\n"
	       "  -w, --windowed          Inverse of --fullscreen.\n"
		   "  -n, --no-vsync					Disable vsync\n"
	       "  -v, --verbose           Enable verbose output\n"
	       "  -q, --quiet             Inverse of --verbose.\n"
				 "  -l, --no-loading        Don't show loading scene (faster load).\n"
	       "  -h, --help              This text\n",
			program_name, FULLSCREEN ? "true" : "false");
#else
	printf(GAME_NAME " " GAME_VERSION " (based on fubar engine)\n"
			"usage: %s [OPTIONS]\n"
			"\n"
			"	-r x y	Set window resolution (default 800x600 in windowed and \n"
			"																current resolution in fullscreen.\n"
			"	-f														Toggle fullscreen: (default: %s)\n"
			" -w														Enable wiimote support\n"
			" -v														Verbose\n",
		program_name, FULLSCREEN ? "on" : "off");
#endif
}

static int fullscreen = FULLSCREEN;
static int vsync = 1;
static int verbose_flag = 0;

#ifndef WIN32
static struct option options[] = {
	{"resolution",   required_argument, 0, 'r'},
	{"fullscreen",   no_argument,       &fullscreen, 1},
	{"windowed",     no_argument,       &fullscreen, 0},
	{"no-vsync",     no_argument,       &vsync, 0},
	{"verbose",      no_argument,       &verbose_flag, 1},
	{"quiet",        no_argument,       &verbose_flag, 0},
	{"help",         no_argument,       0, 'h'},
	{0,0,0,0} /* sentinel */
};
#endif

int main(int argc, char* argv[]){
	/* extract program name from path. e.g. /path/to/MArCd -> MArCd */
	const char* separator = strrchr(argv[0], '/');
	if ( separator ){
		program_name = separator + 1;
	} else {
		program_name = argv[0];
	}

	/* parse arguments */
	int next_index = 1;
#ifndef WIN32
	int op, option_index;
	while ( (op = getopt_long(argc, argv, "r:fwnvqlh", options, &option_index)) != -1 ){
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

		case 'n': /* --no-vsync */
			vsync = 0;
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
			util_abort();
		}
	};
	next_index = optind;
#else // WIN32
	for (int i = 0; i < argc; ++i)
	{
		const char* arg = argv[i];
		++next_index;
		if (strcmp(arg, "-f") == 0)
			fullscreen = !fullscreen;
		else if (strcmp(arg, "-v") == 0)
			verbose_flag = 1;
		else if (strcmp(arg, "-w") == 0)
			useWII = 1;
		else if(strcmp(arg, "-r") == 0) {
			resolution.x = atoi(argv[++i]);
			resolution.y = atoi(argv[++i]);
			next_index+=2;
			resolution_given = true;
		}
		else --next_index;
	}
#endif

	if(argc > next_index) {
		level = std::string(argv[next_index]);
	}

#ifndef WIN32
	verbose = fopen(verbose_flag ? "/dev/stderr" : LOGFILE, "w");
#else
	verbose = (verbose_flag) ? stderr : fopen(LOGFILE, "w");
#endif

	if(!verbose) {
		fprintf(stderr, "Failed to open logfile: %s\n", strerror(errno));
		verbose = stderr;
	}

	/* proper termination */
	signal(SIGINT, handle_sigint);

#ifndef WIN32
	if(verbose_flag) {
		/* setup FPS alarm handler */
		itimerval difftime;
		difftime.it_interval.tv_sec = 1;
		difftime.it_interval.tv_usec = 0;
		difftime.it_value.tv_sec = 1;
		difftime.it_value.tv_usec = 0;
		signal(SIGALRM, show_fps);
		setitimer(ITIMER_REAL, &difftime, NULL);
	}
#endif

	init(fullscreen, vsync);
	main_loop();
	cleanup();
	
	fclose(verbose);

	return 0;
}
