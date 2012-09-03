#include "game.hpp"

#include <glm/gtx/vector_angle.hpp>
#include <SDL/SDL.h>
#include "globals.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"
#include "shader.hpp"
#include "quad.hpp"
#include "terrain.hpp"
#include "color.hpp"
#include "data.hpp"
#include "render_object.hpp"
#include "utils.hpp"
#include "input.hpp"
#include "sound.hpp"

#define DEBUG_MOVE 0

void Game::init() {
}

Game::Game(const std::string &level) : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 100.f) {

	screen = new RenderTarget(resolution, GL_RGB8, false);
	composition = new RenderTarget(resolution, GL_RGB8, true);

	printf("Loading level %s\n", level.c_str());

	lights.ambient_intensity() = glm::vec3(.02f);
	lights.num_lights() = 1;

	lights.lights[0]->set_position(glm::vec3(1.0, 1.0, 0.0f));
	lights.lights[0]->intensity = glm::vec3(0.8f);
	lights.lights[0]->type = Light::POINT_LIGHT;
	lights.lights[0]->constant_attenuation = 0.0f;
	lights.lights[0]->linear_attenuation = 0.1f;
	lights.lights[0]->quadratic_attenuation = 0.4f;

	camera.set_position(glm::vec3(0, 0.2, 0));
	camera.look_at(model->position());
}

Game::~Game() {
	delete composition;
	delete screen;
	/*for(RenderTarget * ds: downsample) {
		delete ds;
	}

	delete dof_shader;*/
}

void Game::update(float dt) {
}

void Game::handle_input(const SDL_Event &event) {
	input.parse_event(event);
}

void Game::render_geometry(const Camera &cam) {

	shaders[SHADER_PASSTHRU]->bind();
	Shader::upload_camera(cam);
	Shader::upload_lights(lights);

	RenderTarget::clear(Color::black);

	shaders[SHADER_NORMAL]->bind();

}

void Game::render() {
	composition->bind();

	render_geometry(camera);

	composition->unbind();

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
	shaders[SHADER_PASSTHRU]->bind();
}
