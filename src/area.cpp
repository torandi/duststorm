#include "area.hpp"
#include "data.hpp"

#include "game.hpp"

static const float marker_size = 0.5f;

Area::Area(const std::string &name, Game &game_) : game(game_) {
	char * str;
	if(asprintf(&str, PATH_BASE "/game/areas/%s.yaml", name.c_str()) == -1) abort();

	Data * src = Data::open(str);
	free(str);

	YAML::Node config = YAML::Load((char*)(src->data()));

	terrain_shader = Shader::create_shader(config["shader"].as<std::string>("terrain"));
	u_fog_density = terrain_shader->uniform_location("fog_density");
	u_marker = terrain_shader->uniform_location("marker_position");


	//Create terrain
	terrain_textures[0] = TextureArray::from_filename(config["textures"][0].as<std::string>().c_str(),config["textures"][1].as<std::string>().c_str(), nullptr);
	terrain_textures[0]->texture_bind(Shader::TEXTURE_ARRAY_0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//terrain_textures[1] = TextureArray::from_filename("dirt_normal.png","grass_normal.png", nullptr);
	terrain_textures[1] = TextureArray::from_filename("default_normalmap.jpg","default_normalmap.jpg", nullptr); //TODO
	terrain_textures[1]->texture_bind(Shader::TEXTURE_ARRAY_0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	terrain = new Terrain(name, config["scale"].as<float>(1.f), config["height"].as<float>(), terrain_textures[0], terrain_textures[1]);

	terrain_shader->bind();
	printf("%f\n", marker_size/terrain->size().x);
	glUniform1f(terrain_shader->uniform_location("marker_size"), marker_size/terrain->size().x);

	fog_density = config["fog"].as<float>(0.02f);

	//TODO: Read from config
	lights.ambient_intensity() = glm::vec3(0.0f);
	lights.num_lights() = 1;
	lights.lights[0].set_position(glm::vec3(10, 50.f, 10.f));
	lights.lights[0].intensity = glm::vec3(0.8f);
	lights.lights[0].type = Light::POINT_LIGHT;
	lights.lights[0].quadratic_attenuation = 0.00002f;

	skycolor = config["skycolor"].as<Color>(Color::red);

	u_highlight = shaders[SHADER_NORMAL]->uniform_location("highlight");
	shaders[SHADER_NORMAL]->bind();
	glUniform3f(u_highlight, 0.f, 0.f, 0.f);
	Shader::unbind();
}

Area::~Area() {
	delete terrain;

	delete terrain_textures[0];
	delete terrain_textures[1];
}

void Area::upload_lights() {
	Shader::upload_lights(lights);
}

void Area::update(float dt) {

}

bool Area::mouse_at(const glm::vec2 &pos) {
	return false;
}

bool Area::click_at(const glm::vec2 &pos) {
	return false;
}


void Area::render(const glm::vec2 &marker_position) {
	RenderTarget::clear(skycolor);
	terrain_shader->bind();
	glUniform1f(u_fog_density, fog_density);
	glUniform2f(u_marker, 1.f - (marker_position.x/terrain->size().x), 1.f - (marker_position.y/terrain->size().y));
	terrain->render();
}

float Area::height_at(const glm::vec2 &pos) const {
	return terrain->get_height_at(pos.x, pos.y);
}

bool Area::collision_at(const glm::vec2 &pos) const {
	return terrain->get_collision_at(pos.x, pos.y);
}
