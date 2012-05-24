#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "globals.hpp"
#include "render_object.hpp"
#include "rendertarget.hpp"
#include "utils.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "time.hpp"

#include "cl.hpp"

#include "particle_system.hpp"

#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unistd.h>
#include <getopt.h>

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;
Time global_time(per_frame);

static volatile bool running = true;
static const char* program_name;
static bool resolution_given = false;

//These are all test variables that should be moved into a scene later
static RenderObject * tv_test;
static Camera * camera;
static Shader::lights_data_t lights;
static Light * light;
static int frames = 0;

static ParticleSystem * particles;

static const char* shader_programs[] = {
	"simple",
	"normal",
	"particles",
	"debug"
};

Scene* scene[0] = {};

static double rotation = 0.0;

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

static void load_shaders() {
	Shader::initialize();
	for(int i=0; i < NUM_SHADERS; ++i) {
		shaders[i] = Shader::create_shader(shader_programs[i]);
	}
}

static void init(bool fullscreen){
	if ( SDL_Init(SDL_INIT_VIDEO) != 0 ){
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		exit(1);	}

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

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	load_shaders();

	tv_test = new RenderObject("models/tv.obj");
	//tv_test->roll(M_PI_4);

	camera = new Camera(75.f, resolution.x/(float)resolution.y, 0.1f, 100.f);
	camera->set_position(glm::vec3(0.f, 0.f, -1.f));
	camera->look_at(glm::vec3(0.f, 0.f, 0.f));

	lights.num_lights = 1;
	lights.ambient_intensity = glm::vec3(0.1, 0.1, 0.1);
	light = new Light(Light::POINT_LIGHT, glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.f, 0.f, 1.f));
	lights.lights[0] = light->shader_light();
	Shader::upload_lights(lights);

	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glDepthRange(0.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	screen_ortho = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
	screen_ortho = glm::scale(screen_ortho, glm::vec3(1.0f, -1.0f, 1.0f));
	screen_ortho = glm::translate(screen_ortho, glm::vec3(0.0f, -600.0f, 0.0f));

	checkForGLErrors("post init()");

	opencl = new CL();

	particles = new ParticleSystem(2000000);
}

static void cleanup(){
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

static void render(){
	glClearColor(1,0,1,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	static int x = 0;
	shaders[SHADER_NORMAL]->bind();

//	shaders[SHADER_NORMAL]->upload_projection_view_matrices(camera->projection_matrix(), camera->view_matrix());


//	tv_test->render(shaders[SHADER_NORMAL]);

	//checkForGLErrors("model render");

	shaders[SHADER_NORMAL]->unbind();

	shaders[SHADER_PARTICLES]->bind();

	Shader::upload_model_matrix(glm::mat4(1.f));
	glDisable(GL_CULL_FACE);
	particles->render();
	checkForGLErrors("Render particles");

	shaders[SHADER_PARTICLES]->unbind();
	
	SDL_GL_SwapBuffers();

	checkForGLErrors("SDL_GL_SwapBuffer");
}

static void update(float dt){
	float t = global_time.get();
	for ( Scene* s: scene ){
		s->update_scene(t, dt);
	}
	tv_test->yaw(M_PI_4*dt);

	rotation += dt*M_PI_4/4.f;
	rotation = fmod(rotation, 2.f*M_PI);

	glm::vec3 pos = glm::vec3(cos(rotation), 0.f, sin(rotation));

	camera->set_position(pos);

	particles->update(dt);

	Shader::upload_camera(*camera);

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
