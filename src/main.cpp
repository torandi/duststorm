#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <signal.h>

static volatile bool running = true;

void handle_sigint(int signum){
	if ( !running ){
		fprintf(stderr, "\rgot SIGINT again, aborting\n");
		abort();
	}

	running = false;
	fprintf(stderr, "\rgot SIGINT, terminating graceful\n");
}

static void init(){

}

static void cleanup(){

}

static void magic_stuff(){
	while ( running ){

	}
}

int main(int argc, char* argv[]){
	signal(SIGINT, handle_sigint);

	init();
	magic_stuff();
	cleanup();

	return 0;
}
