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

static RenderObject * test_object;

void Game::init() {
}

Game::Game(const std::string &level) : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 100.f) {

	screen = new RenderTarget(resolution, GL_RGB8, false);
	composition = new RenderTarget(resolution, GL_RGB8, true);

	printf("Loading level %s\n", level.c_str());

	std::string base_dir = PATH_BASE "data/levels/" + level;

	TextureArray * colors = TextureArray::from_filename( (base_dir +"/color0.png").c_str(),
			(base_dir + "/color1.png").c_str(), nullptr);
	TextureArray *  normals = TextureArray::from_filename( (base_dir +"/normal0.png").c_str(),
			(base_dir + "/normal1.png").c_str(), nullptr);

	terrain = new Terrain(base_dir + "/map.png", 0.5, 50.0, colors, normals);

	lights.ambient_intensity() = glm::vec3(0.1f);
	lights.num_lights() = 1;

	lights.lights[0]->set_position(glm::vec3(0.0, 100.0f, 0.0f));
	lights.lights[0]->intensity = glm::vec3(0.8f);
	lights.lights[0]->type = Light::DIRECTIONAL_LIGHT;
	/*lights.lights[0]->quadratic_attenuation = 0.00002f;
	lights.lights[0]->constant_attenuation = 0.0f;
	lights.lights[0]->linear_attenuation = 0.1f;
	lights.lights[0]->quadratic_attenuation = 0.4f;*/

	camera.set_position(glm::vec3(256.0, 35.0, 256));
	camera.look_at(glm::vec3(0.0, 0.5, 0.0));

	test_object = new RenderObject("pony1.obj");
	test_object->set_position(glm::vec3(1.0, 0, 1.0));

	terrain_shader = Shader::create_shader("terrain");
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
	input.update_object(camera, dt);
	if(input.current_value(Input::ACTION_0) > 0.5f) {
		printf("Current position: (%f, %f, %f)\n", camera.position().x, camera.position().y, camera.position().z);
	}
}

void Game::handle_input(const SDL_Event &event) {
	input.parse_event(event);
}

void Game::render_geometry(const Camera &cam) {

	shaders[SHADER_PASSTHRU]->bind();
	Shader::upload_camera(cam);
	Shader::upload_lights(lights);

	shaders[SHADER_NORMAL]->bind();
	test_object->render();

	terrain_shader->bind();
	terrain->render();

}

void Game::render() {
	composition->bind();

	RenderTarget::clear(Color::black);

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
