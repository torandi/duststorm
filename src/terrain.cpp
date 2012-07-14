#include "terrain.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "globals.hpp"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define RENDER_DEBUG 0

Terrain::~Terrain() {
	if(map_ != NULL)
		delete map_;
	if(terrain_data_ != NULL)
		delete terrain_data_;
	SDL_FreeSurface(data_map_);
	delete data_;
}

Terrain::Terrain(const std::string &name, float horizontal_scale, float vertical_scale,TextureArray * color_, TextureArray * normal_) :
		horizontal_scale_(horizontal_scale),
		vertical_scale_(vertical_scale),
		map_(nullptr),
		base_(name)
		{
	textures_[0] = color_;
	textures_[1] = normal_;
	shader_ = Shader::create_shader("terrain");

	data_map_  = TextureBase::load_image(base_ + "_data.png", &dm_size);
	heightmap_ = TextureBase::load_image(base_ + "_map.png", &hm_size);
	terrain_data_ = Texture2D::from_filename(base_ + "_data.png");
	height_texture_ = Texture2D::from_filename(base_ + "_map.png");
	generate_terrain();	
	SDL_FreeSurface(heightmap_);

	data_ = new unsigned char[dm_size.x*dm_size.y];
	for(int i = 0; i < dm_size.x * dm_size.y; ++i) {
		data_[i] = DATA_UNDEFINED;
	}

	//absolute_move(-glm::vec3(hm_size.x*horizontal_scale_, 0, height_*horizontal_scale_)/2.0f);
}

Texture2D * Terrain::heightmap() const {
	return height_texture_;
}

const glm::ivec2 &Terrain::size() const {
	return hm_size;
}

Texture2D * Terrain::datamap() const { return terrain_data_; }

void Terrain::generate_terrain() {
	unsigned long numVertices = hm_size.x*hm_size.y;

	map_ = new float[numVertices];
	wall_ = new float[numVertices];

	fprintf(verbose,"Generating terrain...\n");
	fprintf(verbose,"World size: %dx%d, scale: %fx%f\n", hm_size.x, hm_size.y, horizontal_scale_, vertical_scale_);

	vertices_ = std::vector<vertex_t>(numVertices);
	for(int y=0; y<hm_size.y; ++y) {
		for(int x=0; x<hm_size.x; ++x) {
			vertex_t v;
			int i = y * hm_size.x + x;
			glm::vec4 color = get_pixel_color(x, y, heightmap_, hm_size);
			float h = height_from_color(color);
			v.position = glm::vec3(horizontal_scale_*x, h*vertical_scale_, horizontal_scale_*y); 
			v.tex_coord = glm::vec2(1.f-v.position.x/(hm_size.x*horizontal_scale_), 1.f-v.position.z/(hm_size.y*horizontal_scale_));
			vertices_[i] = v;
			map_[i] =  h*vertical_scale_;
			wall_[i] = color.g;
			if(color.b > 0.9f) {
				spawnmap.push_back(glm::ivec2(x, y));
			}
		}
	}
	unsigned long indexCount = (hm_size.y - 1 ) * (hm_size.x -1) * 6;

	//build indices
	indices_ = std::vector<unsigned int>(indexCount);
	for(int x=0; x<hm_size.x- 1; ++x) {
		for(int y=0; y<hm_size.y- 1; ++y) {
			int i = y * (hm_size.x-1) + x;
			indices_[i*6 + 2] = (x + 1) + y*hm_size.x;
			indices_[i*6 + 1] = x + (y+1)*hm_size.x;
			indices_[i*6 + 0] = x + y*hm_size.x;

			indices_[i*6 + 5] = (x + 1) + y*hm_size.x;
			indices_[i*6 + 4] = (x+1) + (y+1)*hm_size.x;
			indices_[i*6 + 3] = x + (y+1)*hm_size.x;
		}
	}
	fprintf(verbose,"Terrain generated, creating mesh\n");

	fprintf(verbose,"Generating normals\n");
	generate_normals();
	fprintf(verbose,"Generating tangents\n");
	generate_tangents_and_bitangents();
	fprintf(verbose,"Ortonormalizing tangent space\n");
	ortonormalize_tangent_space();
	fprintf(verbose,"Uploading to gfx memory\n");
	generate_vbos();
}

float Terrain::height_from_color(const glm::vec4 &color) const {
	return color.r;
}

bool Terrain::get_collision_at(int x, int y) const {
	int pos = y*dm_size.x + x;
	if(data_[pos] == DATA_UNDEFINED) {
		glm::vec4 c = get_pixel_color(x, y, data_map_, dm_size);
		data_[pos] = c.g > 0.9f;
	}
	return data_[pos];
}

bool Terrain::get_collision_at(float x, float y) const {
	//Calculate target pixel:
	int _x = (int)round(x*(dm_size.x/(float)hm_size.x));
	int _y = (int)round(y*(dm_size.y/(float)hm_size.y));
	return get_collision_at(_x, _y);
}

float Terrain::get_wall_at(int x, int y) const {
	return wall_[y*hm_size.x + x];
}

float Terrain::get_height_at(int x, int y) const {
	return map_[y*hm_size.x + x];
}

float Terrain::get_height_at(float x_, float y_) const {
	int x = (int) (x_/horizontal_scale_);
	int y = (int) (y_/horizontal_scale_);
	float dx = (x_/horizontal_scale_) - x;
	float dy = (y_/horizontal_scale_) - y;
	float height=0;
	height += (1.0-dx) * (1.0-dy) * map_[y*hm_size.x + x];
	height += dx * (1.0-dy) * map_[y*hm_size.x + x+1];
	height += (1.0-dx) * dy * map_[(y+1)*hm_size.x + x];
	height += dx * dy * map_[(y+1)*hm_size.x + x+1];
	return height;
}

glm::vec4 Terrain::get_pixel_color(int x, int y, SDL_Surface * surface, const glm::ivec2 &size) {
	glm::ivec4 c = TextureBase::get_pixel_color(x, y, surface, size);
	glm::vec4 color;
	color.r = (float)c.x/0xFF;
	color.g = (float)c.y/0xFF;
	color.b = (float)c.z/0xFF;
	color.a = (float)c.w/0xFF;

	return color;	
}

void Terrain::render() {
	Shader::upload_model_matrix(matrix());

	terrain_data_->texture_bind(Shader::TEXTURE_2D_0);
	textures_[0]->texture_bind(Shader::TEXTURE_ARRAY_0);
	textures_[1]->texture_bind(Shader::TEXTURE_ARRAY_1);

	Mesh::render();

#if RENDER_DEBUG
	//Render debug:
	glLineWidth(2.0f);
	shaders[SHADER_DEBUG]->bind();


	Mesh::render();
#endif
}
