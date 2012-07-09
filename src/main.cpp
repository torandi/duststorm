#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#define GAME_NAME "Teh Game"

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
#include <unistd.h>
#include <getopt.h>
#include <map>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define LOGFILE PATH_BASE "frob.log"

glm::mat4 screen_ortho;           /* orthographic projection for primary fbo */

static volatile bool running = true;
static const char* program_name;
static bool resolution_given = false;
static int frames = 0;

static void poll();

void terminate() {
	running = false;
}

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

static void init(bool fullscreen, bool vsync){
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

	if(vsync) SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_SetVideoMode(resolution.x, resolution.y, 0, SDL_OPENGL|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
	SDL_EnableKeyRepeat(0, 0);
	SDL_WM_SetCaption(GAME_NAME, NULL);

	//if ( fullscreen ){
		SDL_ShowCursor(SDL_DISABLE);
	//}

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	Engine::setup_opengl();
	Engine::load_shaders();
	Shader::initialize();

	screen_ortho = glm::ortho(0.0f, (float)resolution.x, 0.0f, (float)resolution.y, -1.0f, 1.0f);
	screen_ortho = glm::scale(screen_ortho, glm::vec3(1.0f, -1.0f, 1.0f));
	screen_ortho = glm::translate(screen_ortho, glm::vec3(0.0f, -(float)resolution.y, 0.0f));

	GLint max_texture_units;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);
	fprintf(verbose, "Supports %d texture units\n", max_texture_units);

	Engine::init();


	checkForGLErrors("post init()");

}

static void cleanup(){
	Engine::cleanup();
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
	Engine::update(dt);
}

static void main_loop(){
	/* for calculating dt */
	struct timeval last;
	gettimeofday(&last, NULL);

	while ( running ){
		poll();

		/* calculate dt */
		struct timeval cur;
		gettimeofday(&cur, NULL);
		float dt = (cur.tv_sec - last.tv_sec)+ (cur.tv_usec - last.tv_usec)/1000000.0 ;

		update(dt);
		render();

		/* move time forward */
		frames++;
		last = cur;

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
				 "  -n, --no-vsync					Disable vsync\n"
	       "  -v, --verbose           Enable verbose output\n"
	       "  -q, --quiet             Inverse of --verbose.\n"
				 "  -l, --no-loading        Don't show loading scene (faster load).\n"
	       "  -h, --help              This text\n",
	       program_name);
}

static int fullscreen = FULLSCREEN;
static int vsync = 1;
static int verbose_flag = 0;

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
			abort();
		}
	};

	verbose = fopen(verbose_flag ? "/dev/stderr" : LOGFILE, "w");

	/* proper termination */
	signal(SIGINT, handle_sigint);

	if(verbose_flag) {
		/* setup FPS alarm handler */
		struct itimerval difftime;
		difftime.it_interval.tv_sec = 1;
		difftime.it_interval.tv_usec = 0;
		difftime.it_value.tv_sec = 1;
		difftime.it_value.tv_usec = 0;
		signal(SIGALRM, show_fps);
		setitimer(ITIMER_REAL, &difftime, NULL);
	}

	
	init(fullscreen, vsync);
	main_loop();
	cleanup();
	

	Engine::init();

	fclose(verbose);

	return 0;
}
