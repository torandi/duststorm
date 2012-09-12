#include "game.hpp"

#include <glm/gtx/string_cast.hpp>

#include <vector>
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
#include "particle_system.hpp"

#include "path.hpp"
#include "rails.hpp"
#include "nanosvg.h"

void Game::init() {
}

Game::Game(const std::string &level) : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 400.f) {

	//This is stuff that should be read from a config
	static float start_position = 0.f;
	sky_color = Color(0.584f, 0.698f, 0.698f, 1.f);
	static glm::vec2 terrain_scale(0.5f, 50.f);
	camera_offset = glm::vec3(0.f, 2.f, -2.5f);
	look_at_offset = 10.f;
	movement_speed = 5.f;


	composition = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER | RenderTarget::DOUBLE_BUFFER);

	printf("Loading level %s\n", level.c_str());

	std::string base_dir = PATH_BASE "data/levels/" + level;

	TextureArray * colors = TextureArray::from_filename( (base_dir +"/color0.png").c_str(),
			(base_dir + "/color1.png").c_str(), nullptr);
	TextureArray *  normals = TextureArray::from_filename( (base_dir +"/normal0.png").c_str(),
			(base_dir + "/normal1.png").c_str(), nullptr);

	terrain = new Terrain(base_dir + "/map.png", terrain_scale.x, terrain_scale.y, colors, normals);

	Data * path_file = Data::open(base_dir + "/path.svg");

	SVGPath * svg_path = svgParse((char*) path_file->data());

	delete path_file;

	std::vector<glm::vec3> path_nodes;

	for(int i=0; i< svg_path->npts; ++i) {
		path_nodes.push_back(glm::vec3(
						svg_path->pts[i*2],
						0,
						svg_path->pts[i*2 + 1]
					) * terrain_scale.x);
	}

	svgDelete(svg_path);
	
	Path::optimize_vector(path_nodes);

	for(glm::vec3 &v : path_nodes) {
		v.y = terrain->height_at(v.x, v.z) + 1.f;
	}

	path = new Path(path_nodes, false);

	rails = new Rails(path, 1.f);
	rail_texture = Texture2D::from_filename(PATH_BASE "data/textures/rails.png");
	rail_material.texture = rail_texture;

//Setup player:
	player.update_position(path, start_position);

//Configure lights:

	lights.ambient_intensity() = glm::vec3(0.1f);
	lights.num_lights() = 1;

	lights.lights[0]->set_position(glm::normalize(glm::vec3(1.0, -1.f, 0.0f)));
	lights.lights[0]->intensity = glm::vec3(0.8f);
	lights.lights[0]->type = MovableLight::DIRECTIONAL_LIGHT;

//Set up camera:

	update_camera();



//Create particle systems:
/*
	particle_textures = TextureArray::from_filename(PATH_BASE "data/textures/fire1.png", 
																	PATH_BASE "data/textures/fire2.png", 
																	PATH_BASE "data/textures/fire3.png", 
																	PATH_BASE "data/textures/smoke.png", 
																	PATH_BASE "data/textures/smoke2.png",
																	nullptr);
*/
	//TODO!
}

Game::~Game() {
	delete composition;
	delete terrain;

	delete path;
	delete rails;
}

void Game::update(float dt) {

	player.update_position(path, player.path_position() + movement_speed * dt);
	update_camera();

	input.update_object(*lights.lights[0], dt);

	if(input.has_changed(Input::ACTION_0, 0.2f) && input.current_value(Input::ACTION_0) > 0.9f) {
		movement_speed -= 1.f;
		printf("Decreased movement speed\n");
	}
	if(input.has_changed(Input::ACTION_1, 0.2f) && input.current_value(Input::ACTION_1) > 0.9f) {
		movement_speed += 1.f;
		printf("Increased movement speed\n");
	}
/*
	if(input.has_changed(Input::ACTION_2, 0.2f) && input.current_value(Input::ACTION_2) > 0.9f) {
		cur_controll = (cur_controll + 1) % num_controllable;
		printf("Switching controll to %s\n", controllable_names[cur_controll]);
	}

	if(input.current_value(Input::ACTION_3) > 0.9f) {
		path_pos+=dt*Input::movement_speed;
		glm::vec3 cur = path->at(path_pos);
		cur.y += 2.f;
		camera.set_position(cur);
		camera.look_at(path->at(path_pos + 10.f));
	}
*/
}

void Game::handle_input(const SDL_Event &event) {
	input.parse_event(event);
}

void Game::render_geometry() {

	terrain->render_geometry();

	rails->render_geometry();

	player.render_geometry();
}

void Game::render() {
	glClear(GL_DEPTH_BUFFER_BIT);

	lights.lights[0]->render_shadow_map(camera, [&]() -> void  {
		render_geometry();
	});


	composition->bind();

	RenderTarget::clear(sky_color);

	Shader::upload_camera(camera);
	Shader::upload_lights(lights);

	terrain->render();

	rail_material.bind();
	rails->render();

	player.render();

	composition->unbind();

	render_display();
}

void Game::render_display() {
	RenderTarget::clear(Color::black);

	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);

	composition->draw(shaders[SHADER_PASSTHRU]);
}

void Game::update_camera() {
	glm::vec4 rotated_offset = player.rotation_matrix() * glm::vec4(camera_offset, 1.f);
	camera.set_position(player.position() + glm::vec3(rotated_offset.x, rotated_offset.y, rotated_offset.z));
	camera.look_at(path->at(player.path_position() + look_at_offset) + camera_offset);
}
