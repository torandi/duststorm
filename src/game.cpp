#include "game.hpp"

#include <SDL/SDL.h>
#include "globals.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"
#include "shader.hpp"
#include "quad.hpp"
#include "terrain.hpp"
#include "color.hpp"
#include "data.hpp"
#include "yaml-helper.hpp"
#include "render_object.hpp"

#include "input.hpp"

#include <dirent.h>

static RenderObject * cube;

Game::Game() : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 150.f), mouse_marker(glm::vec2(1.0f, -1.f), false), show_mouse_marker(true) {

	camera.set_position(glm::vec3(41.851830, 14.846499, 15.102712));
	camera.look_at(glm::vec3(43.761303, 5.710563, 28.003969));

	mouse_position = glm::vec4(43.761303, 5.7, 28.003969, 1.f);

	cube = new RenderObject("cube.obj");
	cube->set_position(glm::vec3(43.761303, 6.510563, 28.003969));

	screen = new RenderTarget(resolution, GL_RGB8, false);
	composition = new RenderTarget(resolution, GL_RGB8, false);

	mouse_marker_texture = Texture2D::from_filename("mouse_marker.png");

	/*downsample[0] = new RenderTarget(resolution/2, GL_RGB8, false);
	downsample[1] = new RenderTarget(resolution/4, GL_RGB8, false);*/

	
	Input::movement_speed = 5.f;

	Data * src = Data::open(PATH_BASE "/game/game.yaml");
	YAML::Node config = YAML::Load((char*)(src->data()));

	load_areas();
	current_area = areas[config["start_area"].as<std::string>()];

	//dof_shader = Shader::create_shader("dof");

	line = Shader::create_shader("color");

	for(bool &a : sustained_action) {
		a = false;
	}

	glGenBuffers(1, &buff);
}

Game::~Game() {
	for(auto it : areas) {
	delete it.second;
	}

	delete composition;
	delete screen;
	/*for(RenderTarget * ds: downsample) {
		delete ds;
	}

	delete dof_shader;*/
	glDeleteBuffers(1, &buff);
}

void Game::update(float dt) {
	input.update_object(camera, dt);
	if(input.current_value(Input::ACTION_1) > 0.5f) {
		printf("Current position: (%f, %f, %f)\n", camera.position().x, camera.position().y, camera.position().z);
	}

	current_area->update(dt);
}

void Game::do_action(int num) {

}

void Game::update_mouse_position(int x, int y) {

	glm::vec3 win;
	win.x = (float)x;
	win.y = (float)(resolution.y - y);
	composition->bind();
	glReadPixels(win.x, win.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win.z);
	composition->unbind();

	glm::mat4 inv = glm::inverse( camera.projection_matrix() * camera.view_matrix()  );
	glm::vec4 mouse_clip = glm::vec4(( (win.x * 2.f )/ resolution.x) - 1.f, ( (win.y * 2.f) / resolution.y) - 1.f , win.z*2.f -1.f, 1.f);
	glm::vec4 mouse_worldspace =  inv * mouse_clip;
	mouse_position = mouse_worldspace/mouse_worldspace.w;
}

void Game::handle_input(const SDL_Event &event) {
	switch(event.type) {
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == config.keymap[Config::MOVE_UP]) sustained_action[MOVE_UP] = true;
			else if(event.key.keysym.sym == config.keymap[Config::MOVE_DOWN]) sustained_action[MOVE_DOWN] = true;
			else if(event.key.keysym.sym == config.keymap[Config::MOVE_LEFT]) sustained_action[MOVE_LEFT] = true;
			else if(event.key.keysym.sym == config.keymap[Config::MOVE_RIGHT]) sustained_action[MOVE_RIGHT] = true;
			else {
				for(int i=0; i<10; ++i) {
					if(config.keymap[Config::ACTION_1 + i] == event.key.keysym.sym) {
						do_action(i);
					}
				}
			}
		case SDL_KEYUP:
			if(event.key.keysym.sym == config.keymap[Config::MOVE_UP]) sustained_action[MOVE_UP] = false;
			else if(event.key.keysym.sym == config.keymap[Config::MOVE_DOWN]) sustained_action[MOVE_DOWN] = false;
			else if(event.key.keysym.sym == config.keymap[Config::MOVE_LEFT]) sustained_action[MOVE_LEFT] = false;
			else if(event.key.keysym.sym == config.keymap[Config::MOVE_RIGHT]) sustained_action[MOVE_RIGHT] = false;
			break;
		case SDL_MOUSEMOTION:
			update_mouse_position(event.motion.x, event.motion.y);
			break;
	}
	input.parse_event(event);
}


void Game::render_content() {
	composition->bind();

	shaders[SHADER_PASSTHRU]->bind();
	Shader::upload_camera(camera);
	current_area->upload_lights();

	render_statics();

	Shader::unbind();
	composition->unbind();
}

void Game::render() {
	render_content();

/*
	//Blur
	RenderTarget* prev = composition;
	for ( int i = 0; i < 2; i++ ){
		Shader::upload_state(downsample[i]->texture_size());
		Shader::upload_projection_view_matrices(downsample[i]->ortho(), glm::mat4());
		downsample[i]->with([prev, i, downsample]() {
			prev->draw(shaders[SHADER_BLUR], glm::ivec2(0,0), downsample[i]->texture_size());
		});
		prev = downsample[i];
	}
*/	
	screen->with(std::bind(&Game::render_composition, this));
	render_display();
}

void Game::render_statics() {
	current_area->render();
	shaders[SHADER_NORMAL]->bind();
	cube->render();
	line->bind();

	const float data[] = {
		mouse_position.x, mouse_position.y, mouse_position.z, 1.f,
		//cube->position().x, cube->position().y, cube->position().z, 1.f,
		//camera.position().x+camera.local_z().x, camera.position().y+camera.local_z().y, camera.position().z+camera.local_z().z, 1.f,
		//mp_far.x, mp_far.y, mp_far.z, 1.f, 
	};

	glBindBuffer(GL_ARRAY_BUFFER, buff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	Shader::upload_model_matrix(glm::mat4(1.f));

	glPointSize(10.f);
	glDrawArrays(GL_POINTS, 0, 1);

	/*glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/

}

void Game::render_display() {
	RenderTarget::clear(Color::magenta);
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
	screen->draw(shaders[SHADER_PASSTHRU]);
}

void Game::render_composition(){
	RenderTarget::clear(Color::black);

	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(screen->ortho(), glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);

	/*downsample[1]->texture_bind(Shader::TEXTURE_2D_2);
	composition->draw(dof_shader);*/
	composition->draw(shaders[SHADER_PASSTHRU]);
}

void Game::load_areas() {
	//areas["park"] = new Area("park", *this);
	std::list<std::string> files;
	dir_content(PATH_BASE "/game/areas", files);
	for(std::string &f : files) {
		size_t dot = f.find(".");
		std::string name = f.substr(0, dot);
		std::string type = f.substr(dot+1);
		if(type == "yaml") {
			fprintf(verbose, "Found area %s\n", name.c_str());
			areas[name] = new Area(name, *this);
		}
	}
}

void Game::dir_content(const char * dir, std::list<std::string> &files) {
	DIR * dp = opendir(dir);
	dirent *dirp;

	if(dir == nullptr) {
		printf("Critical error: Couldn't open directory %s for reading\n", dir);
		abort();
	}

	while ((dirp = readdir(dp)) != nullptr) {
		files.push_back(std::string(dirp->d_name));
	}
}
