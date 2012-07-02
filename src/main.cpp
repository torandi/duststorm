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

#include "light.hpp"

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;
Time global_time(per_frame);
glm::mat4 screen_ortho;

static int skip_load_scene = 0;

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

static bool loading = true;
static double loading_time = 0.f;
static Texture2D* loading_textures[3];
static Shader * loading_shader;
static Quad * loading_quad[2];
static GLint u_fade;
static float loading_fade = 1.f;
static RenderTarget * loading_blend[2];


static void render_loading_scene() {
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());

	frames++;

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	if(loading_time < 1.f) {
		glUniform1f(u_fade, 1.f);
		loading_textures[0]->texture_bind(Shader::TEXTURE_COLORMAP);
		loading_quad[0]->render();
	}

	float fade;

	if(loading) {
		fade = (float)std::min((float)loading_time,1.f);
	} else {
		fade = (float)std::max(2.f - (float)loading_time,0.f);
	}

	glUniform1f(u_fade, fade);
	loading_textures[1]->texture_bind(Shader::TEXTURE_COLORMAP);
	loading_quad[0]->render();

	loading_textures[2]->texture_bind(Shader::TEXTURE_COLORMAP);
	loading_quad[1]->render();

	SDL_GL_SwapBuffers();
}

/**
 * Render the loading screen
 */
static void prepare_loading_scene() {
	fprintf(verbose, "Preparing loading scene\n");

	loading_textures[0] = Texture2D::from_filename("frob_nocolor.png");
	loading_textures[1] = Texture2D::from_filename("frob_color.png");
	loading_textures[2] = Texture2D::from_filename("loading.png");

	loading_quad[0] = new Quad(glm::vec2(1.f, -1.f), false);
	loading_quad[1] = new Quad(glm::vec2(1.f, -1.f), false);

	float scale = resolution.x/1280.f;

	loading_quad[0]->set_scale(glm::vec3(1024*scale,512*scale,1));
	loading_quad[0]->set_position(glm::vec3(resolution.x/2.f - (1024*scale)/2.f, 3.f*resolution.y/10.f - (512*scale)/2.f,1.f));

	loading_quad[1]->set_scale(glm::vec3(512*scale,128*scale,1));
	loading_quad[1]->set_position(glm::vec3(resolution.x/2.f - (512*scale)/2.f, 7.f*resolution.y/10.f - (128*scale)/2.f,1.f));

	loading_time = 0;

};

static void do_loading_scene() {
	if(skip_load_scene)
		return;

	loading_shader = Shader::create_shader("loading");
	u_fade = loading_shader->uniform_location("fade");

	loading_shader->bind();
	/* for calculating dt */
	struct timeval t, last;
	gettimeofday(&t, NULL);
	last = t;

	while(running && ( ( loading && loading_time < 1.f) || (!loading && loading_time < 2.0f))) {
		poll();
		/* calculate dt */
		struct timeval cur;
		gettimeofday(&cur, NULL);
		const uint64_t delta = (cur.tv_sec - t.tv_sec) * 1000000 + (cur.tv_usec - t.tv_usec);
		const  int64_t delay = per_frame - delta;

		loading_time += 1.0/framerate;

		render_loading_scene();


		/* move time forward */
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
	delete loading_shader;
}

static void free_loading() {
	for(Texture2D *t : loading_textures) {
		delete t;
	}
	for(Quad * q : loading_quad) {
		delete q;
	}
	//delete loading_shader;
}

static void init(bool fullscreen, bool vsync, double seek){
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

	/* show configuration */
	fprintf(verbose, PACKAGE_NAME"-"VERSION"\n"
	        "Configuration:\n"
	        "  Demo: " NAME " (" TITLE ")\n"
	        "  Data path: %s\n"
	        "  Resolution: %dx%d (%s)\n"
#ifdef ENABLE_INPUT
	        "  Input is enabled\n"
#endif
	        , PATH_BASE, resolution.x, resolution.y, fullscreen?"fullscreen":"windowed");

	if(vsync) SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_SetVideoMode(resolution.x, resolution.y, 0, SDL_OPENGL|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
	SDL_EnableKeyRepeat(0, 0);
	SDL_WM_SetCaption(TITLE, NULL);

	if ( fullscreen ){
		SDL_ShowCursor(SDL_DISABLE);
	}

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	Engine::setup_opengl();
	Shader::initialize();

	screen_ortho = glm::ortho(0.0f, (float)resolution.x, 0.0f, (float)resolution.y, -1.0f, 1.0f);
	screen_ortho = glm::scale(screen_ortho, glm::vec3(1.0f, -1.0f, 1.0f));
	screen_ortho = glm::translate(screen_ortho, glm::vec3(0.0f, -(float)resolution.y, 0.0f));

	//Start loading screen:
	prepare_loading_scene();
	do_loading_scene();

	Engine::autoload_scenes();
	Engine::load_shaders();

	GLint max_texture_units;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);
	fprintf(verbose, "Supports %d texture units\n", max_texture_units);

	opencl = new CL();

	/* Preload common textures */
	fprintf(verbose, "Preloading textures\n");
	Texture2D::preload("default.jpg");
	Texture2D::preload("default_normalmap.jpg");

	Engine::init();

	//Stop loading scene
	loading = false;
	do_loading_scene();

	//Wait
	free_loading();

	Engine::start(seek);
	global_time.set_paused(false); /* start time */
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

#ifdef ENABLE_INPUT
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
#endif /* ENABLE_INPUT */
		}

#ifdef ENABLE_INPUT
		input.parse_event(event);
#endif /* ENABLE_INPUT */
	}
}

static void render(){
	checkForGLErrors("Frame begin");

	Engine::render();

	SDL_GL_SwapBuffers();
	checkForGLErrors("Frame end");
}

static void update(float dt){
	float t = global_time.get();
	Engine::update(t, dt);
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
	printf(NAME " ("PACKAGE_NAME"-" VERSION ")\n"
	       "usage: %s [OPTIONS]\n"
	       "\n"
	       "  -r, --resolution=SIZE   Set window resultion (default: 800x600 in windowed and\n"
	       "                          current resolution in fullscreen.)\n"
	       "  -f, --fullscreen        Enable fullscreen mode (default: false)\n"
	       "  -w, --windowed          Inverse of --fullscreen.\n"
				 "	-s, --seek=time					Seek to the given time\n"
				 "  -n, --no-vsync					Disable vsync\n"
	       "  -v, --verbose           Enable verbose output\n"
	       "  -q, --quiet             Inverse of --verbose.\n"
				 "  -l, --no-loading        Don't show loading scene (faster load).\n"
	       "  -h, --help              This text\n",
	       program_name);
}

static int fullscreen = 0;
static int vsync = 1;
static int verbose_flag = 0;

static struct option options[] = {
	{"resolution",   required_argument, 0, 'r'},
	{"fullscreen",   no_argument,       &fullscreen, 1},
	{"windowed",     no_argument,       &fullscreen, 0},
	{"seek" ,				 required_argument, 0, 's'},
	{"no-vsync",     no_argument,       &vsync, 0},
	{"verbose",      no_argument,       &verbose_flag, 1},
	{"quiet",        no_argument,       &verbose_flag, 0},
	{"no-loading",   no_argument,       &skip_load_scene, 1},
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

	double seek=0.0;

	/* parse arguments */
	int op, option_index;
	while ( (op = getopt_long(argc, argv, "r:fws:nvqlh", options, &option_index)) != -1 ){
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

		case 'l':
			skip_load_scene = 1;
			break;

		case 'w': /* --windowed */
			fullscreen = 0;
			break;

		case 's': /* --seek */
			seek = atof(optarg);
			printf("Seek to %lf\n", seek);
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

	verbose = fopen(verbose_flag ? "/dev/stderr" : "/dev/null", "w");

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

	/* let the magic begin */
	init(fullscreen, vsync, seek);
	magic_stuff();
	cleanup();

	fclose(verbose);

	return 0;
}
