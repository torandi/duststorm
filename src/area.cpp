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

	float hscale = config["scale"].as<float>(1.f);
	terrain = new Terrain(name, hscale, config["height"].as<float>(), terrain_textures[0], terrain_textures[1]);

	terrain_shader->bind();
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

	float wall_scale = config["walls"]["scale"].as<float>(1.f);
	glm::vec2 texture_scale = config["walls"]["texture_scale"].as<glm::vec2>(glm::vec2(1.f));
	wall_texture = Texture2D::from_filename(config["walls"]["texture"].as<std::string>());

	//Create walls:
	std::vector<Mesh::vertex_t> vertices;
	std::vector<unsigned int> indices;
	for(int y=0; y < terrain->size().y; ++y) {
		for(int x=0; x < terrain->size().x; ++x) {
			float w = terrain->get_wall_at(x, y)*wall_scale;
			glm::vec2 uv_x( fmod(x*texture_scale.x, 1.f), 0.f);
			glm::vec2 uv_y(0.f, fmod(y*texture_scale.y, 1.f));
			if( w > 0.1f ) {
				Mesh::vertex_t v;
				int index = vertices.size();

				if(terrain->get_wall_at(x, y-1) <= 0.1f) {
					//Face

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y), y*hscale);
					v.tex_coord = uv_x + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_x + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y), y*hscale);
					v.tex_coord = uv_x + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);


					//Face

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y)+w, y*hscale);
					v.tex_coord = uv_x + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y), y*hscale);
					v.tex_coord = uv_x + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_x + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

				}

				if(terrain->get_wall_at(x, y+1) <= 0.1f) {
					//Face


					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y+1), (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, (y+1))+w, (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, (y+1)), (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);
					//Face


					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, (y+1))+w, (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, (y+1)), (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(1, 0)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, (y+1))+w, (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

				}

				if(terrain->get_wall_at(x-1, y) <= 0.1f) {
					//Face
					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y), y*hscale);
					v.tex_coord = uv_y + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					//Face
					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1)+w, (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);
				}

				if(terrain->get_wall_at(x+1, y) <= 0.1f) {
					//Face

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y), y*hscale);
					v.tex_coord = uv_y + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					//Face
					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y+1)+w, (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

				}

				//Face

				v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
				v.tex_coord = uv_x + uv_y + glm::vec2(1, 0.f)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1)+w, (y+1)*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(1, 1)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y)+w, y*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(0, 0.f)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);


				//Face

				v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y+1)+w, (y+1)*hscale);
				v.tex_coord = uv_x + uv_y + glm::vec2(0, 1)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y)+w, y*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(0, 0.f)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1)+w, (y+1)*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(1, 1)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);
			}
		}
	}


	wall = new Mesh(vertices, indices);
	wall->generate_normals();
	wall->generate_tangents_and_bitangents();
	wall->ortonormalize_tangent_space();
	wall->generate_vbos();
	
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

	wall_texture->texture_bind(Shader::TEXTURE_COLORMAP);
	shaders[SHADER_PASSTHRU]->bind();
	wall->render();
}

float Area::height_at(const glm::vec2 &pos) const {
	return terrain->get_height_at(pos.x, pos.y);
}

bool Area::collision_at(const glm::vec2 &pos) const {
	return terrain->get_collision_at(pos.x, pos.y);
}
