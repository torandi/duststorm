#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

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
#include "hitting_particles.hpp"
#include "enemy_template.hpp"
#include "enemy.hpp"

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
	camera(75.f, resolution.x/(float)resolution.y, 0.1f, 400.f),
	accum_unspawned(0),
	player_level(1.f)
{

	composition = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER | RenderTarget::DOUBLE_BUFFER);
	geometry = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER);

	// hud initialization
	hud_static_elements_tex = Texture2D::from_filename(PATH_BASE "/data/textures/hudStatic.png");
	hud_static_elements = new Quad();
	hud_static_elements->set_scale(glm::core::type::vec3(resolution.x,resolution.y,0));


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

	spawn_area_start = config["/environment/spawn_area/start"]->as_float();
	spawn_area_end = config["/environment/spawn_area/end"]->as_float();
	spawn_area_size = spawn_area_end - spawn_area_start;
	spawn_distance = config["/environment/spawn_area/distance"]->as_float();
	despawn_distance = config["/environment/spawn_area/despawn_distance"]->as_float();

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

	// Gets max elevation for a radius around a point to avoid the rails cliping into the ground.
	for(glm::vec3 &v : path_nodes) {
		v.y = glm::max(glm::max(glm::max(terrain->height_at(v.x+1.5f, v.z),terrain->height_at(v.x-1.5f, v.z)),terrain->height_at(v.x, v.z+1.5f)),terrain->height_at(v.x, v.z-1.5f)) + 0.2f;
	}

	path = new Path(path_nodes, false);

	rails = new Rails(path, 1.f);
	rail_texture = Texture2D::from_filename(PATH_BASE "data/textures/rails.png");
	rail_material.texture = rail_texture;

	//Setup player:
	player.update_position(path, start_position);
	player.canon_offset = config["/player/canon_offset"]->as_vec3();
	player.canon_length = config["/player/canon_length"]->as_float();

	//Configure lights:

	lights.ambient_intensity() = config["/environment/light/ambient"]->as_vec3();
	lights.num_lights() = 1;

	lights.lights[0]->set_position(glm::normalize(glm::vec3(1.0, -1.f, 0.0f)));
	lights.lights[0]->intensity = config["/environment/light/sunlight"]->as_vec3();
	lights.lights[0]->type = MovableLight::DIRECTIONAL_LIGHT;

	//Load enemies:
	EnemyTemplate::init(Config::parse(base_dir + "/enemies.cfg"), this);

//Set up camera:

	update_camera();


	
	

//Create particle systems:

	wind_velocity = glm::vec4(config["/environment/wind_velocity"]->as_vec3(), 1.f);
	gravity = glm::vec4(config["/environment/gravity"]->as_vec3(), 1.f);

	particle_shader = Shader::create_shader("particles");

	static const Config particle_config = Config::parse(base_dir + "/particles.cfg");

	static const float canon_inner_radius = particle_config["/particles/spawn_radius"]->as_float();

	particle_textures = TextureArray::from_filename( PATH_BASE "data/textures/smoke.png",
																	PATH_BASE "data/textures/fog.png",
																	PATH_BASE "data/textures/particle.png",
																	nullptr);

	static const int max_attack_particles = particle_config["/particles/max_attack_particles"]->as_int();
	static const int max_smoke_particles = particle_config["/particles/max_smoke_particles"]->as_int();
	static const int max_dust_particles = particle_config["/particles/max_dust_particles"]->as_int();
	static const int max_explosion_particles = particle_config["/particles/max_explosion_particles"]->as_int();

	attack_particles = new HittingParticles(max_attack_particles, particle_textures, EnemyTemplate::max_num_enemies, false);
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

	change_particles(HEAVY_PARTICLES);

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
	dust->config.avg_spawn_velocity = glm::vec4(particle_config["/particles/dust/avg_spawn_velocity"]->as_vec3(), 0);
	dust->avg_spawn_rate = particle_config["/particles/dust/avg_spawn_rate"]->as_float();
	dust->spawn_rate_var = particle_config["/particles/dust/spawn_rate_var"]->as_float();
	dust_spawn_ahead = particle_config["/particles/dust/spawn_ahead"]->as_float();
	half_dust_spawn_area = glm::vec3(dust->config.spawn_area.x, dust->config.spawn_area.y, dust->config.spawn_area.z) / 2.f;

	dust->config.spawn_position = glm::vec4(player.position() - half_dust_spawn_area, 1.f);
	dust->update_config();
	dust->spawn(dust->avg_spawn_rate * 5.0);
	dust->config.spawn_position += glm::vec4(path->at(player.path_position() + dust_spawn_ahead / 2.f), 0.f);
	dust->spawn(dust->avg_spawn_rate * 5.0);

	//Explosions
	explosions = new ParticleSystem(max_explosion_particles, particle_textures, true);

	read_particle_config(particle_config["/particles/explosions"], explosions->config);

	explosions->config.gravity = gravity;
	explosions->config.wind_velocity = wind_velocity;
	explosion_count = particle_config["/particles/explosions/count"]->as_int();
	explosions->config.spawn_area = particle_config["/particles/explosions/spawn_area"]->as_vec4();
	explosions->config.avg_spawn_velocity = glm::vec4(particle_config["/particles/explosions/avg_spawn_velocity"]->as_vec3(), 0);
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

	update_enemies(dt);

	//input.update_object(*lights.lights[0], dt);
	//input.update_object(camera, dt);

	if(input.has_changed(Input::ACTION_0, 0.2f) && input.current_value(Input::ACTION_0) > 0.9f) {
		shoot();
	}
	if(input.has_changed(Input::ACTION_1, 0.2f) && input.current_value(Input::ACTION_1) > 0.9f) {
		Input::movement_speed -= 1.f;
		printf("Decreased movement speed\n");
	}
	if(input.has_changed(Input::ACTION_2, 0.2f) && input.current_value(Input::ACTION_2) > 0.9f) {
		Input::movement_speed += 1.f;
		printf("Increased movement speed\n");
	}
	if(input.has_changed(Input::ACTION_3, 0.2f) && input.current_value(Input::ACTION_3) > 0.9f) {
		printf("Change partciles!\n");
		change_particles((particle_type_t) ((current_particle_type + 1) % 3));
	}

	if (useWII) {
		player.set_canon_pitch(WII->getPitch());
		player.set_canon_yaw(-1 * WII->getRoll());
		
		if (WII->getButtonBPressed()) {
			shoot();
			//WII->setRumble(true);
		}
		else {
			//WII->setRumble(false);
		}
	}
	else {
		player.set_canon_pitch(input.current_value(Input::MOVE_Z) * -90.f);
		player.set_canon_yaw(input.current_value(Input::MOVE_X) * 90.f);
	}

	smoke->update(dt);
	attack_particles->update(dt, enemies, this);

	dust->config.spawn_position = glm::vec4(path->at(player.path_position() + dust_spawn_ahead) - half_dust_spawn_area, 1.f);
	dust->update_config();
	dust->update(dt);

	explosions->update(dt);
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

void Game::update_enemies(float dt) {
	//Despawn old enemies:
	for(auto it = enemies.begin(); it != enemies.end(); ) {
		if((*it)->hp <= 0 ) {
			it = enemies.erase(it);
		} else if(player.path_position() - (*it)->path_position > despawn_distance) {
			it = enemies.erase(it);
		} else {
			++it;
		}
		
	}

	//Start by spawning:
	accum_unspawned += EnemyTemplate::spawn_rate * player_level * dt * current_movement_speed;
	int fail_count = 0;

	float path_pos = player.path_position() + spawn_distance;

	glm::vec3 spawn_base = path->at(path_pos);
	glm::vec3 spawn_side = rails->perpendicular_vector_at(path_pos);
	spawn_side.y = 0;
	spawn_side = glm::normalize(spawn_side);

	while(enemies.size() < EnemyTemplate::max_num_enemies && accum_unspawned > EnemyTemplate::min_spawn_cost && fail_count < 3) {
		int index = floor(frand() * EnemyTemplate::templates.size());
		auto it = EnemyTemplate::templates.begin() + index;
		if(it->spawn_cost <= accum_unspawned && it->min_level <= player_level) {
			float dir = frand() < 0.5 ? -1 : 1;
			glm::vec3 pos = spawn_base + dir * (spawn_area_start + spawn_area_size * frand()) * spawn_side;
			pos.y = terrain->height_at(pos.x, pos.z);
			accum_unspawned -= it->spawn_cost;
			enemies.push_back(it->spawn(pos, path_pos, player_level));
		} else {
			++fail_count;
		}
	}

	for(Enemy * e : enemies) {
		e->update(dt);
	}
}

void Game::handle_input(const SDL_Event &event) {
	input.parse_event(event);
}

void Game::render_geometry() {

	terrain->render_geometry();

	rails->render_geometry();

	player.render_geometry();

	for(const Enemy * e : enemies) {
		e->render_geometry();
	}
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

	shaders[SHADER_NORMAL]->bind();
	for(Enemy * e : enemies) {
		e->render();
	}

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

	// Here the hud will be! Fun fun fun fun!
	
	hud_static_elements_tex->texture_bind(Shader::TEXTURE_2D_0);
	shaders[SHADER_PASSTHRU]->bind();
	
	hud_static_elements->render();
}

void Game::update_camera() {
	glm::vec4 rotated_offset = player.rotation_matrix() * glm::vec4(camera_offset, 1.f);
	camera.set_position(player.position() + glm::vec3(rotated_offset.x, rotated_offset.y, rotated_offset.z));
	camera.look_at(path->at(player.path_position() + look_at_offset) + camera_offset);
}

void Game::shoot() {
	glm::vec3 base_speed = player.direction() * current_movement_speed;
	attack_particles->config.avg_spawn_velocity = glm::vec4(player.aim_direction() * particle_types[current_particle_type].spawn_speed + base_speed, 1.f);
	glm::vec4 spawn_position = glm::vec4(player.position() + player.canon_offset + player.aim_direction() * player.canon_length, 0.f);
	attack_particles->config.spawn_position = spawn_position;
	attack_particles->config.spawn_velocity_var = player.aim_matrix() * particle_types[current_particle_type].config.spawn_velocity_var;
	attack_particles->config.extra = particle_types[current_particle_type].damage;
	attack_particles->spawn(particle_types[current_particle_type].count);

	smoke->config.avg_spawn_velocity = glm::vec4(base_speed + player.aim_direction() * smoke_spawn_speed + glm::vec3(0.f, 1.f, 0.f), 1.f);
	smoke->config.spawn_position = spawn_position;
	smoke->spawn(smoke_count);
}

const Player &Game::get_player() const {
	return player;
}

void Game::change_particles(Game::particle_type_t new_type) {
	current_particle_type = new_type;
	attack_particles->config = particle_types[new_type].config;
}

void Game::enemy_impact(const glm::vec3 &position) {
	explosions->config.spawn_position = glm::vec4(position, 1.f);
	explosions->update_config();
	explosions->spawn(1000.0);
}
