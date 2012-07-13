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

#include <dirent.h>

std::map<std::string, object_template_create*> Game::object_templates;

#define DEBUG_MOVE 1

void Game::init() {
	object_templates["door"] = &Door::create;
	object_templates["decoration"] = &Decoration::create;
	object_templates["pickup"] = &Pickup::create;
}

Game::Game() : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 100.f), current_area(nullptr) {

	screen = new RenderTarget(resolution, GL_RGB8, false);
	composition = new RenderTarget(resolution, GL_RGB8, true);

	mouse_marker_texture = Texture2D::from_filename("mouse_marker.png");

	/*downsample[0] = new RenderTarget(resolution/2, GL_RGB8, false);
	downsample[1] = new RenderTarget(resolution/4, GL_RGB8, false);*/

	Input::movement_speed = 15.f;

	//Prepare objects:

	Data * src = Data::open(PATH_BASE "/game/game.yaml");
	YAML::Node config = YAML::Load((char*)(src->data()));

	player = new Player(config["player"], *this);
	camera_offset = config["camera_offset"].as<glm::vec3>(glm::vec3(3.f));

	load_areas();
	YAML::Node start = config["start"];

	change_area(start["area"].as<std::string>(), start["entry_point"].as<std::string>());

	for(bool &a : sustained_action) {
		a = false;
	}
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
	current_area = get_area(area);
	glm::vec2 pos = current_area->get_entry_point(entry_point);
	printf("ENTRY POINT %f, %f\n", pos.x, pos.y);
	player->move_to(pos);
	look_at_player();
}

void Game::update(float dt) {
	player->update(dt);
	current_area->update(dt);

	if(sustained_action[MOUSE_1] && !current_area->click_at(mouse_position)) {
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

void Game::look_at_player() {
	camera.look_at(player->position());
	camera.set_position(player->position() + camera_offset);
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
			if(event.button.button == SDL_BUTTON_LEFT)
				sustained_action[MOUSE_1] = true;
			else if(event.button.button == SDL_BUTTON_RIGHT)
				sustained_action[MOUSE_2] = true;
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
	player->render();
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
