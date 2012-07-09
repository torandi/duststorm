#include "game.hpp"

#include <SDL/SDL.h>
#include "globals.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"
#include "shader.hpp"
#include "quad.hpp"
#include "terrain.hpp"
#include "color.hpp"

#include "input.hpp"

Game::Game() : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 100.f) {

	camera.set_position(glm::vec3(35.750710, 17.926385, 6.305542));
	camera.look_at(glm::vec3(35.750710, 17.926385, 7.305542));

	composition = new RenderTarget(resolution, GL_RGB8, false);
	screen = new RenderTarget(resolution, GL_RGB8, false);
	downsample[0] = new RenderTarget(glm::ivec2(200, 200), GL_RGB8, false, GL_LINEAR);
	downsample[1] = new RenderTarget(glm::ivec2(100, 100), GL_RGB8, false, GL_LINEAR);
	downsample[2] = new RenderTarget(glm::ivec2( 50, 50), GL_RGB8, false, GL_LINEAR);

	printf("Current position: (%f, %f, %f)\n", camera.position().x, camera.position().y, camera.position().z);
	
	Input::movement_speed = 5.f;

	load_areas();
	current_area = areas["park"];
}

Game::~Game() {
	for(auto it : areas) {
	delete it.second;
	}

	delete composition;
	for(RenderTarget * ds: downsample) {
		delete ds;
	}
}

void Game::update(float dt) {
	input.update_object(camera, dt);
	if(input.current_value(Input::ACTION_1) > 0.5f) {
		printf("Current position: (%f, %f, %f)\n", camera.position().x, camera.position().y, camera.position().z);
	}

	current_area->update(dt);
}

void Game::handle_input(const SDL_Event &event) {
	input.parse_event(event);
}


void Game::render() {
	composition->bind();

	shaders[SHADER_PASSTHRU]->bind();
	Shader::upload_camera(camera);
	current_area->upload_lights();

	render_statics();

	Shader::unbind();
	composition->unbind();

	//Blur
	
	RenderTarget* prev = composition;
	for ( int i = 0; i < 2; i++ ){
		Shader::upload_state(downsample[i]->texture_size());
		Shader::upload_projection_view_matrices(downsample[i]->ortho(), glm::mat4());
		downsample[i]->with([this, prev,i](){
				prev->draw(shaders[SHADER_BLUR], glm::ivec2(0,0), downsample[i]->texture_size());
			});
		prev = downsample[i];
	}
	

	render_display();
}

void Game::render_statics() {
	current_area->render();
}

void Game::render_display() {
	screen->with([this]() {
		this->render_composition();
	});
	RenderTarget::clear(Color::magenta);
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
	screen->draw(shaders[SHADER_PASSTHRU]);
}

void Game::render_composition(){
	RenderTarget::clear(Color::black);

	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(composition->ortho(), glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);

	//downsample[1]->texture_bind(Shader::TEXTURE_2D_2);
	composition->draw(shaders[SHADER_PASSTHRU]);
}

void Game::load_areas() {
	areas["park"] = new Area("park", *this);
}
