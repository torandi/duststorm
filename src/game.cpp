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
#include "yaml-helper.hpp"
#include "render_object.hpp"
#include "utils.hpp"
#include "input.hpp"
#include "sound.hpp"

#include <dirent.h>

std::map<std::string, object_template_create*> Game::object_templates;
std::map<std::string, enemy_create*> Game::enemy_creators;

#define DEBUG_MOVE 0

static Quad * bar; 
static Texture2D * bar_texture;

void Game::init() {
	object_templates["door"] = &Door::create;
	object_templates["decoration"] = &Decoration::create;
	object_templates["pickup"] = &Pickup::create;

	enemy_creators["basic"] = &BasicAI::create;
}

Game::Game() : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 100.f), current_area(nullptr) {

	screen = new RenderTarget(resolution, GL_RGB8, false);
	composition = new RenderTarget(resolution, GL_RGB8, true);

	mouse_marker_texture = Texture2D::from_filename("mouse_marker.png");

	/*downsample[0] = new RenderTarget(resolution/2, GL_RGB8, false);
	downsample[1] = new RenderTarget(resolution/4, GL_RGB8, false);*/

	Input::movement_speed = 15.f;

	//Prepare objects:

	YAML::Node config = YAML::LoadFile(PATH_BASE "/game/game.yaml");

	YAML::Node sfx_config = YAML::LoadFile(PATH_BASE "/game/sfx.yaml");

	for(auto it = sfx_config.begin(); it != sfx_config.end(); ++it) {
		std::string name = it->first.as<std::string>();
		std::string file = it->second.as<std::string>();
		sfx[name] = file;
		printf("Added sfx %s: %s\n", name.c_str(), file.c_str());
	}

	for(auto it = config["pickups"].begin(); it != config["pickups"].end(); ++it) {
		std::string name = it->first.as<std::string>();
		pickup_t p;
		p.vfx = it->second["vfx"].as<std::string>();
		VFX::get_vfx(p.vfx); //preload
		p.radius = it->second["radius"].as<float>();
		p.attribute = it->second["attribute"].as<std::string>();
		p.sfx = it->second["sfx"].as<std::string>();
		p.effect = it->second["effect"].as<int>();

		pickups[name] = p;
	}

	player = new Player(config["player"], *this);
	camera_offset = config["camera_offset"].as<glm::vec3>(glm::vec3(3.f));

	player->last_sfx_score = 0;

	load_areas();
	YAML::Node start = config["start"];

	change_area(start["area"].as<std::string>(), start["entry_point"].as<std::string>());

	for(bool &a : sustained_action) {
		a = false;
	}

	bar = new Quad();
	bar_texture = Texture2D::from_filename("bar.png");

	mix_shader = Shader::create_shader("mix");
	u_texture_mix = mix_shader->uniform_location("texture_mix");
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
}

Sound * Game::play_sfx(const std::string &str, float delay, int loops) {
	auto it = sfx.find(str);
	if(it == sfx.end()) {
		printf("Missing sfx %s\n", str.c_str());
		abort();
	}
	Sound * s = new Sound(it->second.c_str(), loops);
	if(delay > 0.f)
		s->set_delay(delay);
	else
		s->play();
	active_sfx.push_back(s);
	return s;
}

Sound * Game::play_sfx_nolist(const std::string &str, float delay, int loops) {
	auto it = sfx.find(str);
	if(it == sfx.end()) {
		printf("Missing sfx %s\n", str.c_str());
		abort();
	}
	Sound * s = new Sound(it->second.c_str(), loops);
	if(delay > 0.f)
		s->set_delay(delay);
	else
		s->play();
	return s;
}

Area * Game::area() const { return current_area; }

Area * Game::get_area(const std::string &str) const {
	auto it = areas.find(str);
	if(it == areas.end()) {
		printf("Unknown area %s\n", str.c_str());
		abort();
	}
	return it->second;
}

void Game::change_area(const std::string &area, const std::string &entry_point) {
	for(bool &a : sustained_action) {
		a = false; //Stop sustained actions;
	}

	player->change_attr("blood", -player->attr("blood"));

	if(area == "end") {
		Sound * s = play_sfx("victory");
		while(s->is_playing()) {
			Sound::update_system();
		}
		printf("Your score: %f\n", player->score);
		exit(0);

	}
	current_area = get_area(area);
	glm::vec2 pos = current_area->get_entry_point(entry_point);
	player->move_to(pos);
	look_at_player();
}

void Game::update(float dt) {
	Sound::update_system();

	if(player->dead == true) {
		Sound * s = play_sfx("gameover");
		while(s->is_playing()) {
			Sound::update_system();
		}
		exit(0);
	}

	active_sfx.remove_if([](const Sound * s) {
			if(s->is_done()) {
				delete s;
				return true;
			};
			return false;
	});

	for(Sound *s : active_sfx) {
		s->update(dt);
	}

	player->update(dt);
	current_area->update(dt);

	if(sustained_action[MOUSE_1]) {
		move_player();
	}

#if DEBUG_MOVE
	input.update_object(camera, dt);
	if(input.current_value(Input::ACTION_1) > 0.5f) {
		printf("Current position: (%f, %f, %f)\n", camera.position().x, camera.position().y, camera.position().z);
	}
#else
	//Move camera:

	look_at_player();
#endif
}

ObjectTemplate * Game::create_object(const std::string &name, const YAML::Node &node, Area * a) { 
	auto it = object_templates.find(name);
	if(it == object_templates.end()) {
		printf("Unknown object type %s\n", name.c_str());
		abort();
	}
	ObjectTemplate * ot = (it->second) (node, *this);
	Area * tmp = current_area;
	if(a != nullptr)
		current_area = a; //Oh the hack :)
	ot->obj->move_to(node["position"].as<glm::vec2>());

	current_area = tmp;
	return ot;
}

Enemy * Game::create_enemy(const YAML::Node &node, const glm::vec2 &pos, Area * a) { 
	std::string name = node["type"].as<std::string>();
	auto it = enemy_creators.find(name);
	if(it == enemy_creators.end()) {
		printf("Unknown enemy type %s\n", name.c_str());
		abort();
	}
	Enemy * e = (it->second) (node, *this);
	Area * tmp = current_area;
	if(a != nullptr)
		current_area = a; //Oh the hack :)
	e->move_to(pos);

	current_area = tmp;
	return e;
}

ObjectTemplate * Game::spawn_pickup(const std::string &name, const glm::vec2 &pos) {
	auto it = pickups.find(name);
	if(it == pickups.end()) {
		printf("Unknown pickup %s\n", name.c_str());
		abort();
	}
	ObjectTemplate * ot = Pickup::create(it->second.vfx, it->second.attribute, it->second.effect, it->second.radius, it->second.sfx, *this);

	ot->obj->move_to(pos);

	return ot;
}

void Game::look_at_player() {
	camera.look_at(player->position());
	camera.set_position(player->position() + camera_offset);
	float height = current_area->height_at(glm::vec2(camera.position().x, camera.position().z));
	if(camera.position().y - height < 1.f)
		camera.absolute_move(glm::vec3(0, (height + 1.f) - camera.position().y, 0.f));
}

//Move player towards mouse position
void Game::move_player() {
	player->target = mouse_position;

	//Rotate:
	player->face(mouse_position);
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
	glm::vec4 mp_v4 = mouse_worldspace/mouse_worldspace.w;
	mouse_position.x = mp_v4.x;
	mouse_position.y = mp_v4.z;
	current_area->mouse_at(mouse_position);
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
		case SDL_MOUSEBUTTONDOWN:
			update_mouse_position(event.button.x, event.button.y);
			if(event.button.button == SDL_BUTTON_LEFT) {
				if(SDL_GetModState() & KMOD_SHIFT) {
					player->face(mouse_position);
					player->swing();
					player->target = player->center();
					player->move_to(player->center());
				} else {
					if(!current_area->click_at(mouse_position, 1)) {
						sustained_action[MOUSE_1] = true;
					}
				}
			} else if(event.button.button == SDL_BUTTON_RIGHT) {
				if(!current_area->click_at(mouse_position, 2)) {
						sustained_action[MOUSE_2] = true;
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			update_mouse_position(event.button.x, event.button.y);
			if(event.button.button == SDL_BUTTON_LEFT)
				sustained_action[MOUSE_1] = false;
			else if(event.button.button == SDL_BUTTON_RIGHT)
				sustained_action[MOUSE_2] = false;
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
	render_dynamics();

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
	mouse_marker_texture->texture_bind(Shader::TEXTURE_2D_1);
	current_area->render(mouse_position);
	shaders[SHADER_NORMAL]->bind();
}

void Game::render_dynamics() {
}

void Game::render_display() {
	RenderTarget::clear(Color::magenta);
	Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
	mix_shader->bind();
	int life = player->attr("life");
	if(life < PLAYER_LOW_HP) {
		float mix = (PLAYER_LOW_HP - (float)life)/PLAYER_LOW_HP;
		glUniform1f(u_texture_mix, mix);
	} else {
		glUniform1f(u_texture_mix, 0.f);
	}
	screen->draw(mix_shader);
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
	bar_texture->texture_bind(Shader::TEXTURE_COLORMAP);
	bar->set_scale(glm::vec3(100.f*((float)player->attr("life")/player->attributes_max["life"]), 30.f, 0.f));
	bar->set_position(glm::vec3(10.f, resolution.y - 30.f, 0.f));
	bar->render();

	bar->set_scale(glm::vec3(100.f*((float)player->attr("blood")/current_area->required_blood), 30.f, 0.f));
	bar->set_position(glm::vec3(resolution.x - 110.f, resolution.y - 30.f, 0.f));
	bar->render();
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
