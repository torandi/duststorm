#include "area.hpp"
#include "data.hpp"

#include "game.hpp"

Area::Area(const std::string &name, Game &game_) : game(game_) {
	char * str;
	if(asprintf(&str, PATH_BASE "/game/areas/%s.yaml", name.c_str()) == -1) abort();

	Data * src = Data::open(str);
	free(str);

	YAML::Node config = YAML::Load((char*)(src->data()));

	terrain_shader = Shader::create_shader(config["shader"].as<std::string>("terrain"));

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

	//TODO: Read from config
	lights.ambient_intensity() = glm::vec3(0.0f);
	lights.num_lights() = 1;
	lights.lights[0].set_position(glm::vec3(10, 50.f, 10.f));
	lights.lights[0].intensity = glm::vec3(0.8f);
	lights.lights[0].type = Light::POINT_LIGHT;
	lights.lights[0].quadratic_attenuation = 0.00002f;

	skycolor = Color::rgb(149.0f / 255.0f, 178.0f / 255.0f, 178.0f / 255.0f);

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

void Area::render() {
	RenderTarget::clear(skycolor);
	terrain_shader->bind();
	terrain->render();
}

float Area::height_at(const glm::ivec2 &pos) const {
	return terrain->get_height_at(pos.x, pos.y);
}
