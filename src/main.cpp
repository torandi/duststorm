#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "render_object.hpp"

#include "rendertarget.hpp"
#include "utils.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "time.hpp"

#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unistd.h>

static const unsigned int framerate = 60;
static const uint64_t per_frame = 1000000 / framerate;

Time global_time(per_frame);      /* current time */
glm::ivec2 resolution;            /* current resolution */
glm::mat4 screen_ortho;           /* orthographic projection for primary fbo */

Shader * shaders[];

static volatile bool running = true;
static RenderTarget* test = nullptr;

static RenderObject * tv_test;
static Camera * camera;

static const char* shader_programs[] = {
	"simple",
	"normal"
};

class TestScene: public Scene {
public:
	TestScene(const glm::ivec2& size): Scene(size){

	}

	virtual void update(float t, float dt){
		printf("test scene active\n");
	}
};

Scene* scene[1] = {0,};

static void handle_sigint(int signum){
	if ( !running ){
		fprintf(stderr, "\rgot SIGINT again, aborting\n");
		abort();
	}

	running = false;
	fprintf(stderr, "\rgot SIGINT, terminating graceful\n");
}

static void load_shaders() {
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
	resolution.x = vi->current_w;
	resolution.y = vi->current_h;

	if ( !fullscreen ){
		resolution.x = 800;
		resolution.y = 600;
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

	tv_test = new RenderObject("models/cube.obj");
	camera = new Camera(75.f, resolution.x/(float)resolution.y, -1.0, 1.0);
	camera->set_position(glm::vec3(0.f, 0.f, -1.f));
	camera->look_at(glm::vec3(0.f, 0.f, 0.f));

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

	scene[0] = new TestScene(resolution);
	scene[0]->add_time(1, 4);
	scene[0]->add_time(7, 10);

	checkForGLErrors("post init()");

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

	test->bind();
	test->clear((x++ % 2 == 0) ? Color::green : Color::blue);
	test->unbind();

	//test->draw();
	//
	shaders[SHADER_NORMAL]->bind();

	shaders[SHADER_NORMAL]->upload_projection_view_matrices(camera->projection_matrix(), camera->view_matrix());

	tv_test->render(shaders[SHADER_NORMAL]);

	checkForGLErrors("model render");

	shaders[SHADER_NORMAL]->unbind();

	SDL_GL_SwapBuffers();

	checkForGLErrors("SDL_GL_SwapBuffer");
}

static void update(float dt){
	float t = global_time.get();
	for ( Scene* s: scene ){
		s->update_scene(t, dt);
	}
}

static void magic_stuff(){
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

		global_time.update();
		update(global_time.dt());
		render();

		/* move time forward */
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

int main(int argc, char* argv[]){
	const bool fullscreen = argc >= 2;
	signal(SIGINT, handle_sigint);

	init(fullscreen);
	magic_stuff();
	cleanup();

	return 0;
}
