#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "game.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

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
#include "highscore.hpp"

#include "path.hpp"
#include "rails.hpp"
#include "nanosvg.h"
#include "config.hpp"

static glm::vec2 hud_scale;

static const Color hud_font_color(0.f,0.64f,1.f, 1.f);
static const Color highscore_color(0.4f,0.70f,1.f, 1.f);

static const float break_factor = 0.3f;
static const float break_duration = 3.0f;
static const float break_cooldown = 10.0f;

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

bool Game::start_pressed() const {
	if(input.has_changed(Input::START, 0.2f) && input.current_value(Input::START) > 0.9f) {
		return true;
	}

#ifdef WIN32
	if(useWII) {
		WII->setRumble(false);
		if (WII->getButtonPlusPressed()) {
			return true;
		}
	}
#endif
	return false;
}

void Game::init() {
}

Game::Game(const std::string &level, float near, float far, float fov) :
	camera(fov, resolution.x/(float)resolution.y, near, far)
	, music(nullptr)
	, current_mode(MODE_READY)
{
	composition = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER | RenderTarget::DOUBLE_BUFFER);
	geometry = new RenderTarget(resolution, GL_RGB8, RenderTarget::DEPTH_BUFFER);

	printf("Loading level %s\n", level.c_str());

	std::string base_dir = PATH_BASE "data/levels/" + level;

	Config config = Config::parse(base_dir + "/level.cfg");

	//Read config:
	start_position = config["/player/start_position"]->as_float();
	sky_color = config["/environment/sky_color"]->as_color();
	static const glm::vec2 terrain_scale = config["/environment/terrain/scale"]->as_vec2();
	static const float fog_intensity = config["/environment/fog_intensity"]->as_float();
	camera_offset = config["/player/camera/offset"]->as_vec3();
	look_at_offset = config["/player/camera/look_at_offset"]->as_float();
	movement_speed = config["/player/speed/normal"]->as_float();
	brake_movement_speed = config["/player/speed/brake"]->as_float();

	spawn_area_start = config["/game/spawn_area/start"]->as_float();
	spawn_area_end = config["/game/spawn_area/end"]->as_float();
	spawn_area_size = spawn_area_end - spawn_area_start;
	spawn_distance = config["/game/spawn_area/distance"]->as_float();
	despawn_distance = config["/game/spawn_area/despawn_distance"]->as_float();
	difficulty_increase = config["/game/difficulty_increase"]->as_float();

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
	player.canon_offset = config["/player/canon_offset"]->as_vec3();
	player.canon_length = config["/player/canon_length"]->as_float();
	//Configure lights:

	lights.ambient_intensity() = config["/environment/light/ambient"]->as_vec3();
	lights.num_lights() = 1;

	lights.lights[0]->intensity = config["/environment/light/sunlight"]->as_vec3();
	lights.lights[0]->type = MovableLight::DIRECTIONAL_LIGHT;

	//Load enemies:
	EnemyTemplate::init(Config::parse(base_dir + "/enemies.cfg"), this);

//Set up camera:

	update_camera();

//Create particle systems:

	wind_velocity = config["/environment/wind_velocity"]->as_vec3();
	gravity = glm::vec4(config["/environment/gravity"]->as_vec3(), 1.f);

	particle_shader = Shader::create_shader("particles");
	passthru = Shader::create_shader("passthru");

	static const Config particle_config = Config::parse(base_dir + "/particles.cfg");

	static const float canon_inner_radius = particle_config["/particles/spawn_radius"]->as_float();

	particle_textures = TextureArray::from_filename( PATH_BASE "data/textures/smoke.png",
																	PATH_BASE "data/textures/fog.png",
																	PATH_BASE "data/textures/particle.png",
																	PATH_BASE "data/textures/fire1.png",
																	PATH_BASE "data/textures/fire2.png",
																	PATH_BASE "data/textures/fire3.png",
																	nullptr);

	static const int max_attack_particles = particle_config["/particles/max_attack_particles"]->as_int();
	static const int max_smoke_particles = particle_config["/particles/max_smoke_particles"]->as_int();
	static const int max_dust_particles = particle_config["/particles/max_dust_particles"]->as_int();
	static const int max_explosion_particles = particle_config["/particles/max_explosion_particles"]->as_int();

	attack_particles = new HittingParticles(max_attack_particles, particle_textures, EnemyTemplate::max_num_enemies, false);
	attack_particles->config.gravity = gravity;
	system_configs.push_back(&(attack_particles->config));
	attack_particles->config.spawn_area = glm::vec4(0.f, 0.f, 0.f, canon_inner_radius);

	for(auto &p : particle_types) {
		p.config = attack_particles->config; //Get reasonable defaults
		system_configs.push_back(&(p.config));
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

	//Smoke:
	smoke = new ParticleSystem(max_smoke_particles, particle_textures, false);
	smoke->config.gravity = gravity;
	system_configs.push_back(&(smoke->config));
	smoke->config.spawn_area = glm::vec4(0.f, 0.f, 0.f, canon_inner_radius * 2.0);

	read_particle_config(particle_config["/particles/smoke"], smoke->config);
	smoke_count = particle_config["/particles/smoke/count"]->as_int();
	smoke_spawn_speed = particle_config["/particles/smoke/spawn_speed"]->as_float();

	//Dust:
	dust = new ParticleSystem(max_dust_particles, particle_textures, true);

	read_particle_config(particle_config["/particles/dust"], dust->config);

	dust->config.gravity = gravity;
	system_configs.push_back(&(dust->config));
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
	explosions = new ParticleSystem(max_explosion_particles, particle_textures, false);

	hit_explosion = explosions->config;
	system_configs.push_back(&(hit_explosion));
	kill_explosion = explosions->config;
	system_configs.push_back(&(kill_explosion));
	read_particle_config(particle_config["/particles/hit_explosion"], hit_explosion);
	read_particle_config(particle_config["/particles/kill_explosion"], kill_explosion);

	explosions->config.gravity = gravity;
	system_configs.push_back(&(explosions->config));
	hit_explosion_count = particle_config["/particles/hit_explosion/count"]->as_int();
	kill_explosion_count = particle_config["/particles/kill_explosion/count"]->as_int();
	kill_explosion.spawn_area = particle_config["/particles/kill_explosion/spawn_area"]->as_vec4();
	kill_explosion.avg_spawn_velocity = glm::vec4(particle_config["/particles/kill_explosion/avg_spawn_velocity"]->as_vec3(), 0);
	hit_explosion.spawn_area = particle_config["/particles/hit_explosion/spawn_area"]->as_vec4();
	hit_explosion.avg_spawn_velocity = glm::vec4(particle_config["/particles/hit_explosion/avg_spawn_velocity"]->as_vec3(), 0);

	update_wind_velocity();
	//Setup HUD




	hud_scale = glm::vec2(resolution.x / 800.f, resolution.y / 600.f);
	hud_static_elements_tex = Texture2D::from_filename(PATH_BASE "/data/textures/hudStatic.png");
	fullscreen_quad = new Quad();
	fullscreen_quad->set_scale(glm::vec3(resolution.x,resolution.y,0));

	hud_lightpos =  glm::vec2(594,490) * hud_scale;
	hud_mediumpos = glm::vec2(644,490) * hud_scale;
	hud_heavypos =  glm::vec2(700,490) * hud_scale;

	hud_choice_tex = Texture2D::from_filename(PATH_BASE "/data/textures/weap_select.png");
	hud_choice_quad = new Quad();
	hud_choice_quad->set_scale(glm::vec3(97,92,0) * glm::vec3(hud_scale , 0));

	hud_break_tex = Texture2D::from_filename(PATH_BASE "/data/textures/breaks_ready.png");
	hud_break_quad = new Quad();
	hud_break_quad->set_scale(glm::vec3(190, 25, 0) * glm::vec3(hud_scale, 0));
	hud_break_quad->set_position(glm::vec3(glm::vec2(30, 530) * hud_scale, 0.f));

	life_text.set_color(hud_font_color);
	life_text.set_scale(20.0 * hud_scale.x);
	life_text.set_position(glm::vec3(glm::vec2(26.f, 44.5f) * hud_scale, 0.f));

	//Highscore stuff:
	highscore = new Highscore(base_dir + "/highscore", NUM_HIGHSCORE_ENTRIES);
	float hs_scale = 40.f;
	glm::vec2 higscore_base = glm::vec2(735.f, 85.f);
	int i=0;
	for(Text &t : highscore_entries) {
		t.set_position(glm::vec3( (higscore_base + glm::vec2(0.f, (i++)*hs_scale)) * hud_scale, 0.f));
		t.set_scale(hs_scale);
		t.set_text("");
		t.set_alignment(Text::RIGHT_ALIGNED);
	}

	//Textures
	game_over_texture = Texture2D::from_filename(PATH_BASE "/data/textures/gameover.png");
	startscreen_texture = Texture2D::from_filename(PATH_BASE "/data/textures/start_screen.png");
}

void Game::initialize() {

	input.reset();

	//Reset light:
	lights.lights[0]->set_position(glm::normalize(glm::vec3(1.0, -1.f, 0.0f)));

	//Set player variables
	player.update_position(path, start_position);
	current_movement_speed = movement_speed;

	accum_unspawned = 0;
	player_level = 0.7f;
	life = 100;
	score = 0;

	last_break = -2 * break_cooldown;

	current_particle_type = MEDIUM_PARTICLES;
	change_particles(0);


	score_text.set_color(hud_font_color);
	score_text.set_alignment(Text::LEFT_ALIGNED);
	life_text.set_number(life);
	score_text.set_number(score);

	//Reset score position:
	score_text.set_scale(20.0 * hud_scale.x);
	score_text.set_position(glm::vec3(glm::vec2(26.f, 70.5f) * hud_scale, 0.f));

	if(!music_mute) {
	delete music;
	music = new Sound("ecstacy.mp3", 1);
	music->play();
	}
}

Game::~Game() {
	for ( auto ptr: enemies ){
		delete ptr;
	}
	enemies.clear();

	delete music;

	delete composition;
	delete geometry;
	delete terrain;

	delete path;
	delete rails;
	delete highscore;

	delete smoke;
	delete attack_particles;
	delete particle_textures;
	delete explosions;
}

void Game::update(float dt) {

	switch(current_mode) {
		case MODE_READY:
			fade_music(dt);
			if(start_pressed()) {
				current_mode = MODE_GAME;
				initialize();
			}
			break;
		case MODE_GAME:
			{
				if(life <= 0) {
					
					//delete music;
					current_mode = MODE_HIGHSCORE;

					highscore->add_entry(score);

					score_text.set_alignment(Text::RIGHT_ALIGNED);
					score_text.set_number(score);
					score_text.set_color(highscore_color);
					score_text.set_scale(30.0 * hud_scale.x);
					score_text.set_position(glm::vec3(glm::vec2(150.f, 270.f) * hud_scale, 0.f));

					int i = 0;
					bool selected = false;
					for(int e : highscore->get_entries()) {
						if(e > 0) {
							if(!selected && e == score) {
								highscore_entries[i].set_color(hud_font_color);
							} else {
								highscore_entries[i].set_color(highscore_color);
							}
							highscore_entries[i].set_number(e);
						} else {
							highscore_entries[i].set_text("");
						}
						++i;
					}
#ifdef WIN32
					if(useWII) WII->setRumble(false);
#endif

					return;
				}

				// Change wind direction
				wind_velocity = glm::rotateY(wind_velocity, (float)(0.8f + sin(global_time)));
				update_wind_velocity();
				
				//input.update_object(camera, dt);

				bool buttonFirePressed = (input.has_changed(Input::ACTION_0, 0.2f) && input.current_value(Input::ACTION_0) > 0.9f);
				bool buttonSwapForwPressed = (input.has_changed(Input::ACTION_3, 0.2f) && input.current_value(Input::ACTION_3) > 0.9f);
				bool buttonSwapBackPressed = (input.has_changed(Input::ACTION_2, 0.2f) && input.current_value(Input::ACTION_2) > 0.9f);
				bool buttonBreakPressed = (input.has_changed(Input::ACTION_1, 0.2f) && input.current_value(Input::ACTION_1) > 0.9f);
				bool buttonMutePressed = false;

#ifdef WIN32
				if (useWII) {
					float pitch = WII->getPitch(), roll = -1 * WII->getRoll(); // [-90, 90].
					pitch = glm::clamp(pitch - 10, -90.0f, 90.0f);
					//pitch = glm::clamp(-10 + 90 * glm::sign(pitch) * glm::pow(glm::abs(1.0f * pitch / 90), 1.0f), -90.0f, 90.0f);
					//roll = glm::clamp(90 * glm::sign(roll) * glm::pow(glm::abs(1.0f * roll / 90), 1.0f), -90.0f, 90.0f);
					player.set_canon_pitch(pitch);
					player.set_canon_yaw(roll);

					buttonFirePressed = WII->getButtonAPressed();
					buttonSwapBackPressed = WII->getArrowLeftPressed();
					
					buttonSwapForwPressed = WII->getArrowRightPressed();
					buttonBreakPressed = WII->getButtonBDown();
					//buttonMutePressed = WII->getArrowUpPressed();
					const bool wiiSwapAB = false;
					if (wiiSwapAB) std::swap(buttonFirePressed, buttonBreakPressed);
				} else {
#endif
					player.set_canon_pitch(input.current_value(Input::MOVE_Y) * 90.f);
					player.set_canon_yaw(input.current_value(Input::MOVE_X) * 90.f);
#ifdef WIN32
				}
#endif
				if (buttonFirePressed) {
					shoot();

#ifdef WIN32
					if(useWII) WII->setRumble(true);
				} else {
					if(useWII) WII->setRumble(false);
#endif

				}
				
				if(buttonMutePressed) {
					music_mute = !music_mute;
					if(music_mute) {
						music ->stop();
					}
					//change_particles(-1);

				}

				if (buttonSwapForwPressed) {
					change_particles(1);
				}
				if (buttonSwapBackPressed) {
					change_particles(-1);
				}

				if( buttonBreakPressed && global_time - last_break > break_cooldown)
					last_break = global_time;

				float speed = current_movement_speed;
				if (global_time - last_break < break_duration)
					speed *= break_factor;



				player.update_position(path, player.path_position() + speed * dt);

				update_camera();

				update_enemies(dt);

				smoke->update(dt);
				attack_particles->update(dt, enemies, this);

				dust->config.spawn_position = glm::vec4(path->at(player.path_position() + dust_spawn_ahead) - half_dust_spawn_area, 1.f);
				dust->update_config();
				dust->update(dt);
				
				explosions->update(dt);

				life_text.set_number(life);
				score_text.set_number(score);

			dust->config.spawn_position = glm::vec4(path->at(player.path_position() + dust_spawn_ahead) - half_dust_spawn_area, 1.f);
			dust->update_config();
			dust->update(dt);
			
			explosions->update(dt);
			
			active_sounds.remove_if([](const Sound * s) {
				if(s->is_done()) {
					delete s;
					return true;
				};
				return false;
			});
			// Really ugly way of looping the music:
			if(music != nullptr && music->is_done() && !music_mute)
			{
				delete music;
				music = new Sound("ecstacy.mp3", 5);
				music->play();
			}
		}
		break;
	case MODE_HIGHSCORE:
		fade_music(dt);
		if(start_pressed()) {
			current_mode = MODE_READY;
		}
		break;
	}

	input.update(dt);
}

void Game::evolve() {
	player_level += difficulty_increase;
}
void Game::fade_music(float dt){
	if(music != nullptr && music->is_playing()) {
			float vol = music->get_volume();
			vol-= dt*0.2f;
			if(vol<0) {
				music->stop();
			} else {
				music->set_volume(vol);
			}
		}
}

void Game::draw_selected_weap()
{
	if(current_particle_type==0)
		hud_choice_quad->set_position(glm::vec3(hud_lightpos,0));
	else if(current_particle_type==1)
		hud_choice_quad->set_position(glm::vec3(hud_mediumpos,0));
	else if(current_particle_type==2)
		hud_choice_quad->set_position(glm::vec3(hud_heavypos,0));
	hud_choice_tex->texture_bind(Shader::TEXTURE_2D_0);
	passthru->bind();
	hud_choice_quad->render();
}

void Game::update_enemies(float dt) {
	for(auto it = enemies.begin(); it != enemies.end(); ) {
		if((*it)->hp <= 0 ) {
			enemy_impact((*it)->position(), true);
			it = enemies.erase(it);
			life += 1;
			score += (int) (player_level * 10.f);
			evolve();
		} else if(player.path_position() - (*it)->path_position > despawn_distance) {
			it = enemies.erase(it);
			life -= 10;
		} else {
			++it;
		}
	}
	life = glm::clamp(life, 0, 200);

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

	if(current_mode == MODE_GAME) {
		lights.lights[0]->render_shadow_map(camera, [&]() -> void  {
			render_geometry();
		});

		geometry->bind();
		geometry->clear(Color::black);
		passthru->bind();
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

		passthru->bind();
		for(Enemy * e : enemies) {
			e->render();
		}

		particle_shader->bind();
		geometry->depth_bind(Shader::TEXTURE_2D_0);

		attack_particles->render();
		
		smoke->render();
		explosions->render();
		dust->render();

		composition->unbind();

	}

	render_display();
}

void Game::render_display() {
	RenderTarget::clear(Color::magenta);

	Shader::upload_state(resolution);
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
	glViewport(0, 0, resolution.x, resolution.y);

	switch(current_mode) {

		case MODE_GAME:
			composition->draw(passthru);

			hud_static_elements_tex->texture_bind(Shader::TEXTURE_2D_0);
			passthru->bind();
			
			fullscreen_quad->render();

			life_text.render();
			score_text.render();

			draw_selected_weap();

			if (global_time - last_break > break_cooldown) {
				hud_break_tex->texture_bind(Shader::TEXTURE_2D_0);
				hud_break_quad->render();
			}

			break;
		case MODE_READY:
			startscreen_texture->texture_bind(Shader::TEXTURE_2D_0);
			passthru->bind();
			fullscreen_quad->render();
			break;
		case MODE_HIGHSCORE:
			game_over_texture->texture_bind(Shader::TEXTURE_2D_0);
			passthru->bind();
			fullscreen_quad->render();
			score_text.render();
			for(Text &e : highscore_entries) {
				e.render();
			}
			break;
	}
}

void Game::update_camera() {
	glm::vec3 rotated_offset = glm::vec3(player.rotation_matrix() * glm::vec4(camera_offset, 1.f));
	camera.set_position(player.position() + rotated_offset);
	camera.look_at(path->at(player.path_position() + look_at_offset) + rotated_offset);
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
	play_sound("Explosion_Fast.wav",1);
}

void Game::update_wind_velocity() {
	glm::vec4 v4 = glm::vec4(wind_velocity, 1.f);
	for(ParticleSystem::config_t * c : system_configs) {
		c->wind_velocity = v4;
	}
}

const Player &Game::get_player() const {
	return player;
}

void Game::change_particles(int delta) {
	int new_type = current_particle_type + delta;
	if(new_type>2)
		current_particle_type=LIGHT_PARTICLES;
	else if(new_type<0)
		current_particle_type=HEAVY_PARTICLES;
	else
		current_particle_type = (particle_type_t)new_type;
	//current_particle_type = new_type;
	attack_particles->config = particle_types[current_particle_type].config;
}

void Game::enemy_impact(const glm::vec3 &position, bool kill) {
	int count;
	if(kill) {
		count = kill_explosion_count;
		explosions->config = kill_explosion;
	} else {
		count = hit_explosion_count;
		explosions->config = hit_explosion;
	}
	explosions->config.spawn_position = glm::vec4(position, 1.f);
	explosions->spawn(count);
}

void Game::play_sound(const char* path, int loops)
{
	Sound * s = new Sound(path, loops);
	s->play();
	active_sounds.push_back(s);
}
