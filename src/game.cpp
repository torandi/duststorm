#include "game.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
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
#include "config.hpp"

static void read_particle_config(const ConfigEntry * config, ParticleSystem::config_t &particle_config) {
	particle_config.birth_color = config->find("birth_color", true)->as_vec4();
	particle_config.death_color = config->find("death_color", true)->as_vec4();
	particle_config.motion_rand = glm::vec4(config->find("motion_rand", true)->as_vec3(), 1.f);
	particle_config.spawn_velocity_var = glm::vec4(config->find("spawn_velocity_var", true)->as_vec3(), 1.f);
	particle_config.avg_ttl = config->find("avg_ttl", true)->as_float();
	particle_config.ttl_var = config->find("ttl_var", true)->as_float();
	particle_config.avg_scale = config->find("avg_scale", true)->as_float();
	particle_config.scale_var = config->find("scale_var", true)->as_float();
	particle_config.avg_scale_change = config->find("avg_scale_change", true)->as_float();
	particle_config.scale_change_var = config->find("scale_change_var", true)->as_float();
	particle_config.avg_rotation_speed = config->find("avg_rotation_speed", true)->as_float();
	particle_config.rotation_speed_var = config->find("rotation_speed_var", true)->as_float();
	particle_config.avg_wind_influence = config->find("avg_wind_influence", true)->as_float();
	particle_config.wind_influence_var = config->find("wind_influence_var", true)->as_float();
	particle_config.avg_gravity_influence = config->find("avg_gravity_influence", true)->as_float();
	particle_config.gravity_influence_var = config->find("gravity_influence_var", true)->as_float();
	particle_config.start_texture = config->find("start_texture", true)->as_int();
	particle_config.num_textures = config->find("num_textures", true)->as_int();
}

void Game::init() {
}

Game::Game(const std::string &level) :
	camera(75.f, resolution.x/(float)resolution.y, 0.1f, 400.f)
{

	composition = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER | RenderTarget::DOUBLE_BUFFER);
	geometry = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER);

	printf("Loading level %s\n", level.c_str());

	std::string base_dir = PATH_BASE "data/levels/" + level;

	Config config = Config::parse(base_dir + "/level.cfg");

	//Read config:
	static const float start_position = config["/player/start_position"]->as_float();
	sky_color = config["/environment/sky_color"]->as_color();
	static const glm::vec2 terrain_scale = config["/environment/terrain/scale"]->as_vec2();
	static const float fog_intensity = config["/environment/fog_intensity"]->as_float();
	camera_offset = config["/player/camera/offset"]->as_vec3();
	look_at_offset = config["/player/camera/look_at_offset"]->as_float();
	movement_speed = config["/player/speed/normal"]->as_float();
	brake_movement_speed = config["/player/speed/brake"]->as_float();

	current_movement_speed = movement_speed;

	static const Shader::fog_t fog = { glm::vec4(sky_color.to_vec3(), 1.f), fog_intensity };
	Shader::upload_fog(fog);

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
		v.y = terrain->height_at(v.x, v.z) + 0.2f;
	}

	path = new Path(path_nodes, false);

	rails = new Rails(path, 1.f);
	rail_texture = Texture2D::from_filename(PATH_BASE "data/textures/rails.png");
	rail_material.texture = rail_texture;

	//Setup player:
	player.update_position(path, start_position);
	player.canon_offset = config["/player/canon_offset"]->as_vec3();

	//Configure lights:

	lights.ambient_intensity() = config["/environment/light/ambient"]->as_vec3();
	lights.num_lights() = 1;

	lights.lights[0]->set_position(glm::normalize(glm::vec3(1.0, -1.f, 0.0f)));
	lights.lights[0]->intensity = config["/environment/light/sunlight"]->as_vec3();
	lights.lights[0]->type = MovableLight::DIRECTIONAL_LIGHT;

//Set up camera:

	update_camera();

//Create particle systems:

	wind_velocity = glm::vec4(config["/environment/wind_velocity"]->as_vec3(), 1.f);
	gravity = glm::vec4(config["/environment/gravity"]->as_vec3(), 1.f);

	particle_shader = Shader::create_shader("particles");

	static const Config particle_config = Config::parse(base_dir + "/particles.cfg");

	static const float canon_inner_radius = particle_config["/particles/spawn_radius"]->as_float();

	particle_textures = TextureArray::from_filename(PATH_BASE "data/textures/fire1.png", 
																	PATH_BASE "data/textures/fire2.png", 
																	PATH_BASE "data/textures/fire3.png", 
																	PATH_BASE "data/textures/smoke.png",
																	PATH_BASE "data/textures/fog.png",
																	nullptr);

	static const int max_attack_particles = particle_config["/particles/max_attack_particles"]->as_int();
	static const int max_smoke_particles = particle_config["/particles/max_smoke_particles"]->as_int();
	static const int max_dust_particles = particle_config["/particles/max_dust_particles"]->as_int();
	attack_particles = new ParticleSystem(max_attack_particles, particle_textures, false);
	attack_particles->config.gravity = gravity;
	attack_particles->config.wind_velocity = wind_velocity;
	attack_particles->config.spawn_area = glm::vec4(0.f, 0.f, 0.f, canon_inner_radius);

	for(auto &p : particle_types) {
		p.config = attack_particles->config; //Get reasonable defaults
	}
	read_particle_config(particle_config["/particles/light"], particle_types[LIGHT_PARTICLES].config);
	particle_types[LIGHT_PARTICLES].count = particle_config["/particles/light/count"]->as_int();
	particle_types[LIGHT_PARTICLES].spawn_speed = particle_config["/particles/light/spawn_speed"]->as_float();
	particle_types[LIGHT_PARTICLES].damage = particle_config["/particles/light/damage"]->as_float();

	read_particle_config(particle_config["/particles/medium"], particle_types[MEDIUM_PARTICLES].config);
	particle_types[MEDIUM_PARTICLES].count = particle_config["/particles/medium/count"]->as_int();
	particle_types[MEDIUM_PARTICLES].spawn_speed = particle_config["/particles/medium/spawn_speed"]->as_float();
	particle_types[MEDIUM_PARTICLES].damage = particle_config["/particles/medium/damage"]->as_float();

	read_particle_config(particle_config["/particles/heavy"], particle_types[HEAVY_PARTICLES].config);
	particle_types[HEAVY_PARTICLES].count = particle_config["/particles/heavy/count"]->as_int();
	particle_types[HEAVY_PARTICLES].spawn_speed = particle_config["/particles/heavy/spawn_speed"]->as_float();
	particle_types[HEAVY_PARTICLES].damage = particle_config["/particles/heavy/damage"]->as_float();

	current_particle_type = MEDIUM_PARTICLES;
	attack_particles->config = particle_types[MEDIUM_PARTICLES].config;

	//Smoke:
	smoke = new ParticleSystem(max_smoke_particles, particle_textures, false);
	smoke->config.gravity = gravity;
	smoke->config.wind_velocity = wind_velocity;
	smoke->config.spawn_area = glm::vec4(0.f, 0.f, 0.f, canon_inner_radius * 2.0);

	read_particle_config(particle_config["/particles/smoke"], smoke->config);
	smoke_count = particle_config["/particles/smoke/count"]->as_int();
	smoke_spawn_speed = particle_config["/particles/smoke/spawn_speed"]->as_float();

	//Dust:
	dust = new ParticleSystem(max_dust_particles, particle_textures, true);

	read_particle_config(particle_config["/particles/dust"], dust->config);

	dust->config.gravity = gravity;
	dust->config.wind_velocity = wind_velocity;
	dust->config.spawn_area = particle_config["/particles/dust/spawn_area"]->as_vec4();
	dust->avg_spawn_rate = particle_config["/particles/dust/avg_spawn_rate"]->as_float();
	dust->spawn_rate_var = particle_config["/particles/dust/spawn_rate_var"]->as_float();
	dust_spawn_ahead = particle_config["/particles/dust/spawn_ahead"]->as_float();
	half_dust_spawn_area = glm::vec3(dust->config.spawn_area.x, dust->config.spawn_area.y, dust->config.spawn_area.z) / 2.f;

	dust->config.spawn_position = glm::vec4(player.position() - half_dust_spawn_area, 1.f);
	dust->spawn(dust->avg_spawn_rate * 5.0);
	dust->config.spawn_position += glm::vec4(path->at(player.path_position() + dust_spawn_ahead / 2.f), 0.f);
	dust->spawn(dust->avg_spawn_rate * 5.0);

	//Load enemies:

	//static const Config enemies_config = Config::parse(base_dir + "/enemies.cfg");
}

Game::~Game() {
	delete composition;
	delete geometry;
	delete terrain;

	delete path;
	delete rails;

	delete smoke;
	delete attack_particles;
	delete particle_textures;
}

void Game::update(float dt) {

	player.update_position(path, player.path_position() + current_movement_speed * dt);
	update_camera();

	//input.update_object(*lights.lights[0], dt);
	input.update_object(camera, dt);

	if(input.has_changed(Input::ACTION_0, 0.2f) && input.current_value(Input::ACTION_0) > 0.9f) {
		shoot();
		printf("FIRE!\n");
	}
	if(input.has_changed(Input::ACTION_1, 0.2f) && input.current_value(Input::ACTION_1) > 0.9f) {
		Input::movement_speed -= 1.f;
		printf("Decreased movement speed\n");
	}
	if(input.has_changed(Input::ACTION_2, 0.2f) && input.current_value(Input::ACTION_2) > 0.9f) {
		Input::movement_speed += 1.f;
		printf("Increased movement speed\n");
	}

	smoke->update(dt);
	attack_particles->update(dt);

	dust->config.spawn_position = glm::vec4(path->at(player.path_position() + dust_spawn_ahead) - half_dust_spawn_area, 1.f);
	dust->update_config();
	dust->update(dt);

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

	geometry->bind();
	geometry->clear(Color::black);
	shaders[SHADER_PASSTHRU]->bind();
	Shader::upload_camera(camera);
	render_geometry();
	geometry->unbind();


	Shader::upload_state(composition->texture_size());
	composition->bind();

	RenderTarget::clear(sky_color);

	Shader::upload_camera(camera);
	Shader::upload_lights(lights);


	terrain->render();

	rail_material.bind();
	rails->render();

	player.render();

	particle_shader->bind();
	geometry->depth_bind(Shader::TEXTURE_2D_0);

	dust->render();
	smoke->render();
	attack_particles->render();

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

void Game::shoot() {
	glm::vec4 rotated_offset = player.rotation_matrix() * glm::vec4(player.canon_offset, 1.f);
	glm::vec3 direction = player.direction();
	attack_particles->config.avg_spawn_velocity = glm::vec4(direction * (current_movement_speed + particle_types[current_particle_type].spawn_speed), 1.f);
	attack_particles->config.spawn_position = glm::vec4(player.position(), 0.f) + rotated_offset;
	attack_particles->config.spawn_velocity_var = player.rotation_matrix() * particle_types[current_particle_type].config.spawn_velocity_var;
	attack_particles->spawn(particle_types[current_particle_type].count);

	smoke->config.avg_spawn_velocity = glm::vec4(direction * (current_movement_speed +smoke_spawn_speed), 1.f);
	smoke->config.spawn_position = glm::vec4(player.position(), 0.f) + rotated_offset;
	smoke->spawn(smoke_count);
}
