#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "render_object.hpp"

#include "rendertarget.hpp"
#include "utils.hpp"
#include "shader.hpp"

#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unistd.h>
#include <sys/time.h>

struct timeval global_time = {0,0};      /* current time */
glm::ivec2 resolution;            /* current resolution */
glm::mat4 screen_ortho;           /* orthographic projection for primary fbo */

Shader shader;

static volatile bool running = true;
static bool paused = false;       /* tell if engine is paused */
static int time_scale = 100;      /* how fast time is flowing in percent*/
static int time_step = 0;         /* single-step */
static RenderTarget* test = nullptr;


static const char* shader_programs[] = {
	"simple"
};

static void handle_sigint(int signum){
	if ( !running ){
		fprintf(stderr, "\rgot SIGINT again, aborting\n");
		abort();
	}

	running = false;
	fprintf(stderr, "\rgot SIGINT, terminating graceful\n");
}

static void load_shaders() {
	shader = Shader::create_shader("simple");
}

static void init(){
	if ( SDL_Init(SDL_INIT_VIDEO) != 0 ){
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		exit(1);	}

	const SDL_VideoInfo* vi = SDL_GetVideoInfo();
	if ( !vi ){ fprintf(stderr, "SDL_GetVideoInfo() failed\n"); abort(); }
	resolution.x = vi->current_w;
	resolution.y = vi->current_h;

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_SetVideoMode(vi->current_w, vi->current_h, 0, SDL_OPENGL|SDL_DOUBLEBUF|SDL_FULLSCREEN);
	SDL_EnableKeyRepeat(0, 0);

	SDL_WM_SetCaption("Speed 100%", NULL);

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	load_shaders();

	RenderObject foo("models/tv.obj");

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	screen_ortho = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
	screen_ortho = glm::scale(screen_ortho, glm::vec3(1.0f, -1.0f, 1.0f));
	screen_ortho = glm::translate(screen_ortho, glm::vec3(0.0f, -600.0f, 0.0f));

	test = new RenderTarget(glm::ivec2(400,100), false);
}

static void cleanup(){
	delete test;
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
				time_scale = paused ? 100 : 0;
				paused = !paused;
				scale_updated = true;
			}

			if ( event.key.keysym.sym == SDLK_PERIOD ){
				paused = true;
				time_scale = 0;
				time_step = 1;
				scale_updated = true;
			}

			if ( event.key.keysym.sym == SDLK_COMMA ){
				time_scale -= 10;
				scale_updated = true;
				paused = false;
			} else if ( event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_p ){
				time_scale += 10;
				scale_updated = true;
				paused = false;
			}

			if ( scale_updated ){
				char title[64];
				sprintf(title, "Speed: %d%%", time_scale);
				SDL_WM_SetCaption(title, NULL);
			}
		}
	}
}

static void render(){
	glClearColor(1,0,1,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	static int x = 0;

	test->bind();
	test->clear((x++ % 2 == 0) ? Color::green : Color::blue);
	test->unbind();

	shader.bind();

	shader.upload_projection_view_matrices(screen_ortho, glm::mat4(1.f));

	test->draw();

	shader.unbind();

	SDL_GL_SwapBuffers();
}

static void update(float dt){

}

static void magic_stuff(){
	static const unsigned int framerate = 60;
	static const uint64_t per_frame = 1000000 / framerate;
	static const float dt = 1.0f / framerate;

	/* for calculating dt */
	struct timeval t;
	gettimeofday(&t, NULL);

	while ( running ){
		poll();

		/* calculate dt */
		struct timeval cur;
		gettimeofday(&cur, NULL);
		const uint64_t delta = (cur.tv_sec - t.tv_sec) * 1000000 + (cur.tv_usec - t.tv_usec);
		const  int64_t delay = per_frame - delta;
		const float scale = (float)time_scale / 100.0f;
		float scaled_dt = dt * scale;
		if ( time_step != 0 ){
			scaled_dt = dt * time_step;
		}

		update(scaled_dt);
		render();

		/* move time forward */
		t.tv_usec += per_frame;
		if ( t.tv_usec > 1000000 ){
			t.tv_usec -= 1000000;
			t.tv_sec++;
		}

		/* move global scaled time forward */
		if ( time_step == 0 ){
			global_time.tv_usec += per_frame * scale;
		} else {
			global_time.tv_usec += per_frame;
		}
		if ( global_time.tv_usec > 1000000 ){
			global_time.tv_usec -= 1000000;
			global_time.tv_sec++;
		}

		/* fixed framerate */
		if ( delay > 0 ){
			usleep(delay);
		}
		time_step = 0;
	}
}

int main(int argc, char* argv[]){
	signal(SIGINT, handle_sigint);

	init();
	magic_stuff();
	cleanup();

	return 0;
}
