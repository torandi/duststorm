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

#define DEBUG_MOVE 0

/*
 * Debug stuff
 */
static RenderObject * objects[10];
static const int num_objects = 3;

static RenderObject * path_marker;

static ParticleSystem * test_system;
static TextureArray * particle_textures;

static const int num_controllable = 2;
static MovableObject * controllable[num_controllable];
static const char * controllable_names[] = { "Camera", "Light" };
static int cur_controll = 0;

static Color sky;

static int const num_particle_configs = 2;
static ParticleSystem::config_t particle_configs[num_particle_configs];
static int current_particle_config = 0;

static glm::vec3 prev;
static float path_pos = 0.f;

static Rails * rails;
static Texture2D * rail_texture;

/*
 * End debug stuff
 */

void Game::init() {
}

Game::Game(const std::string &level) : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 400.f) {

	sky = Color(0.584f, 0.698f, 0.698f, 1.f);

	static glm::vec2 terrain_scale(0.5f, 50.f);

	screen = new RenderTarget(resolution, GL_RGB8);
	composition = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER);
	//downsample[0] = new RenderTarget(resolution/2, GL_RGB8, false);
	//downsample[1] = new RenderTarget(resolution/4, GL_RGB8, false);

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

	prev = path->at(0.f);

	lights.ambient_intensity() = glm::vec3(0.1f);
	lights.num_lights() = 2;

	lights.lights[0]->set_position(glm::normalize(glm::vec3(1.0, 0.f, 0.0f)));
	lights.lights[0]->intensity = glm::vec3(0.8f);
	lights.lights[0]->type = MovableLight::DIRECTIONAL_LIGHT;

	lights.lights[1]->set_position(glm::normalize(path->at(0.f)));
	lights.lights[1]->intensity = glm::vec3(0.8f);
	lights.lights[1]->type = MovableLight::POINT_LIGHT;
	/*lights.lights[0]->quadratic_attenuation = 0.00002f;
	lights.lights[0]->constant_attenuation = 0.0f;
	lights.lights[0]->linear_attenuation = 0.1f;
	lights.lights[0]->quadratic_attenuation = 0.4f;*/

	camera.set_position(correct_height(path->at(0.f), 1.f));
	camera.look_at(camera.position() + glm::vec3(1.f, 0.f, 0.f));
	//camera.look_at(correct_height(path->at(1.f)));

	objects[0] = new RenderObject("pony1.obj");
	objects[0]->set_position(correct_height(path->at(5.f) + glm::vec3(-5.f, 0.f, 0.f)));

	objects[1] = new RenderObject("bench.obj");
	objects[1]->set_position(correct_height(path->at(6.f) + glm::vec3(-5.f, 0.f, 0.f)));

	objects[2] = new RenderObject("cube.obj");
	objects[2]->set_scale(0.25);
	objects[2]->add_position_callback(lights.lights[1]);
	objects[2]->set_position(correct_height(path->at(6.f) + glm::vec3(-5.f, 0.f, 0.f)));

	path_marker = new RenderObject("cube.obj");
	path_marker->set_scale(0.25f);

	terrain_shader = Shader::create_shader("terrain");

	//Particles:
	particle_textures = TextureArray::from_filename(PATH_BASE "data/textures/fire1.png", 
																	PATH_BASE "data/textures/fire2.png", 
																	PATH_BASE "data/textures/fire3.png", 
																	PATH_BASE "data/textures/smoke.png", 
																	PATH_BASE "data/textures/smoke2.png",
																	nullptr);
	
	test_system = new ParticleSystem(10000, particle_textures, false);

	controllable[0] = &camera;
	controllable[1] = objects[2];

	//Build particle configs:
	test_system->config.spawn_position = glm::vec4(objects[1]->position(), 1.f);
	test_system->config.spawn_area = glm::vec4(0.0f, 0.f, 0.0f, 1.f);
	test_system->config.avg_spawn_velocity = glm::vec4(0, 0.2f, 0.f, 0.f);
	test_system->config.spawn_velocity_var = glm::vec4(0.f, 0.f, 0.f, 0.f);
	test_system->config.avg_ttl = 10.f;
	test_system->config.ttl_var = 2.f;
	test_system->config.avg_scale = 2.f;
	test_system->config.scale_var = 0.5f;
	test_system->config.avg_scale_change = 4.f;
	test_system->config.scale_change_var = 0.5f;
	test_system->config.avg_rotation_speed = 0.02f;
	test_system->config.rotation_speed_var = 0.01f;
	test_system->config.birth_color = glm::vec4(0.3, 0.3, 0.3, 0.1);
	test_system->config.death_color = glm::vec4(0.8 ,0.8, 0.8, 0.f);
	test_system->config.motion_rand = glm::vec4(0.01f, 0.01f, 0.01f, 0);
	test_system->config.avg_wind_influence = 0.06f;
	test_system->config.wind_influence_var = 0.05f;
	test_system->config.avg_gravity_influence = 0.005f;
	test_system->config.gravity_influence_var = 0.f;
	test_system->config.wind_velocity = glm::vec4(1.f, 0.f, 0.f, 0.f);
	test_system->config.start_texture = 3;
	test_system->config.num_textures = 1;
	
	test_system->update_config();

	test_system->auto_spawn = true;
	
}

Game::~Game() {
	delete composition;
	delete screen;
	for(int i=0; i < num_objects; ++i) {
		delete objects[i];
	}
	delete test_system;
	delete particle_textures;
	delete terrain;

	delete path;
	delete rails;

	/*for(RenderTarget * ds: downsample) {
		delete ds;
	}*/
}

glm::vec3 Game::correct_height(glm::vec3 v, float offset) const {
	return glm::vec3(v.x, terrain->height_at(v.x, v.z) + offset, v.z);
//	return glm::vec3(v.x, 50.f + offset, v.z);
}

void Game::update(float dt) {
	input.update_object(*(controllable[cur_controll]), dt);

	if(input.has_changed(Input::ACTION_0, 0.2f) && input.current_value(Input::ACTION_0) > 0.9f) {
		Input::movement_speed -= 1.f; 
		printf("Decreased movement speed\n");
	}
	if(input.has_changed(Input::ACTION_1, 0.2f) && input.current_value(Input::ACTION_1) > 0.9f) {
		Input::movement_speed += 1.f; 
		printf("Increased movement speed\n");
	}

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
		printf("%f\n", path_pos);

	}

	test_system->update(dt);
	//printf("Current position: (%f, %f, %f)\n", camera.position().x, camera.position().y, camera.position().z);
}

void Game::handle_input(const SDL_Event &event) {
	input.parse_event(event);
}

void Game::render_geometry(const Camera &cam) {

	shaders[SHADER_PASSTHRU]->bind();
	Shader::upload_camera(cam);
	Shader::upload_lights(lights);

	shaders[SHADER_NORMAL]->bind();
	for(int i=0; i < num_objects; ++i) {
		objects[i]->render();
	}

	/*for(float i = 0.f; i<path->length(); i+=1.f) {
		path_marker->set_position(path->at(i));
		path_marker->set_position(correct_height(path->at(i), 1.f));
		path_marker->render();
	}*/

	rail_material.activate();
	rails->render();

	//shaders[SHADER_DEBUG]->bind();
	//rails->render();

	rail_material.deactivate();

	terrain_shader->bind();
	terrain->render();

}

void Game::render() {
	glClear(GL_DEPTH_BUFFER_BIT);
	composition->bind();

	RenderTarget::clear(sky);

	render_geometry(camera);

	/*shaders[SHADER_PARTICLES]->bind();
	test_system->render();*/

	composition->unbind();

	//Blur
	/*RenderTarget* prev = composition;
	for ( int i = 0; i < 2; i++ ){
		Shader::upload_state(downsample[i]->texture_size());
		Shader::upload_projection_view_matrices(downsample[i]->ortho(), glm::mat4());
		downsample[i]->with([prev, i, downsample]() {
			prev->draw(shaders[SHADER_BLUR], glm::ivec2(0,0), downsample[i]->texture_size());
		});
		prev = downsample[i];
	}*/

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
}
