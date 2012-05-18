#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rendertarget.hpp"

#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unistd.h>

static volatile bool running = true;

static glm::mat4 ortho;
static RenderTarget* test = nullptr;

void handle_sigint(int signum){
	if ( !running ){
		fprintf(stderr, "\rgot SIGINT again, aborting\n");
		abort();
	}

	running = false;
	fprintf(stderr, "\rgot SIGINT, terminating graceful\n");
}

static void init(){
	if ( SDL_Init(SDL_INIT_VIDEO) != 0 ){
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_SetVideoMode(800, 600, 0, SDL_OPENGL|SDL_DOUBLEBUF);
	SDL_EnableKeyRepeat(0, 0);

	int ret;
	if ( (ret=glewInit()) != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(ret));
		exit(1);
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	ortho = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
	ortho = glm::scale(ortho, glm::vec3(1.0f, -1.0f, 1.0f));
	ortho = glm::translate(ortho, glm::vec3(0.0f, -600.0f, 0.0f));

	test = new RenderTarget(glm::ivec2(100,100), false);
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

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glLoadMatrixf(glm::value_ptr(ortho));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, test->texture());
	static const float vertices[][5] = { /* x,y,z,u,v */
		{-100, -100, 0, 0, 0},
		{ 100, -100, 0, 1, 0},
		{ 100,  100, 0, 1, 1},
		{-100,  100, 0, 0, 1},
	};
	static const unsigned int indices[4] = {0,1,2,3};
	glVertexPointer  (3, GL_FLOAT, sizeof(float)*5, &vertices[0][0]);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[0][3]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, indices);

	SDL_GL_SwapBuffers();
}

static void magic_stuff(){
	while ( running ){
		poll();
		render();

		usleep(500000);
	}
}

int main(int argc, char* argv[]){
	signal(SIGINT, handle_sigint);

	init();
	magic_stuff();
	cleanup();

	return 0;
}
