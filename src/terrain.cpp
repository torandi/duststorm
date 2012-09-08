#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils.hpp"
#include "terrain.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define RENDER_DEBUG 0

Terrain::~Terrain() {
	if(map_ != NULL)
		delete map_;
	if(data_texture_ != NULL)
		delete data_texture_;
	free_surface();
}

Terrain::Terrain(const std::string &file, float horizontal_scale, float vertical_scale,TextureArray * color_, TextureArray * normal_) :
		horizontal_scale_(horizontal_scale),
		vertical_scale_(vertical_scale),
		map_(nullptr) {
	textures_[0] = color_;
	textures_[1] = normal_;

	for(TextureArray * ta : textures_) {
		ta->texture_bind(Shader::TEXTURE_ARRAY_0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		ta->texture_unbind();
	}

	data_map_  = TextureBase::load_image(file , &size_);
	data_texture_ = Texture2D::from_filename(file);

	shader_ = Shader::create_shader("terrain");
	material.specular = glm::vec4(0.f);

	generate_terrain();
}

void Terrain::free_surface() {
	if(data_map_ != nullptr) {
		SDL_FreeSurface(data_map_);
		data_map_ = nullptr;
	}
}

const glm::ivec2 &Terrain::size() const {
	return size_;
}

void Terrain::generate_terrain() {
	unsigned long numVertices = size_.x*size_.y;

	map_ = new float[numVertices];

	fprintf(verbose,"Generating terrain...\n");
	fprintf(verbose,"World size: %dx%d, scale: %fx%f\n", size_.x, size_.y, horizontal_scale_, vertical_scale_);

	vertices_ = std::vector<vertex_t>(numVertices);
	for(int y=0; y<size_.y; ++y) {
		for(int x=0; x<size_.x; ++x) {
			vertex_t v;
			int i = y * size_.x + x;
			glm::vec4 color = get_pixel_color(x, y, data_map_, size_);
			float h = height_from_color(color);
			v.position = glm::vec3(horizontal_scale_*x, h*vertical_scale_, horizontal_scale_*y); 
			v.tex_coord = glm::vec2((float)x/size_.x, 1.f-(float)y/size_.y);
			vertices_[i] = v;
			map_[i] =  h*vertical_scale_;
		}
	}
	unsigned long indexCount = (size_.y - 1 ) * (size_.x -1) * 6;

	//build indices
	indices_ = std::vector<unsigned int>(indexCount);
	for(int x=0; x<size_.x- 1; ++x) {
		for(int y=0; y<size_.y- 1; ++y) {
			int i = y * (size_.x-1) + x;
			indices_[i*6 + 2] = (x + 1) + y*size_.x;
			indices_[i*6 + 1] = x + (y+1)*size_.x;
			indices_[i*6 + 0] = x + y*size_.x;

			indices_[i*6 + 5] = (x + 1) + y*size_.x;
			indices_[i*6 + 4] = (x+1) + (y+1)*size_.x;
			indices_[i*6 + 3] = x + (y+1)*size_.x;
		}
	}

	generate_normals();
	generate_tangents_and_bitangents();
	ortonormalize_tangent_space();
	generate_vbos();
}

float Terrain::height_from_color(const glm::vec4 &color) const {
	return color.r;
}

float Terrain::height_at(int x, int y) const {
	return map_[y*size_.x + x];
}

float Terrain::height_at(float x_, float y_) const {
	if(x_ > size_.x * horizontal_scale_|| x_ < 0 || y_ > size_.y*horizontal_scale_ || y_ < 0)
		return 0;
	int x = (int) (x_/horizontal_scale_);
	int y = (int) (y_/horizontal_scale_);
	float dx = (x_/horizontal_scale_) - x;
	float dy = (y_/horizontal_scale_) - y;
	float height=0;
	/*height += (1.0-dx) * (1.0-dy) * height_at(x,y);
	height += dx * (1.0-dy) * height_at(y,x+1);
	height += (1.0-dx) * dy * height_at(y+1,x);
	height += dx * dy * height_at(y+1, x+1);*/
	height += (1.0-dx) * (1.0-dy) * map_[y*size_.x + x];
	height += dx * (1.0-dy) * map_[y*size_.x + x+1];
	height += (1.0-dx) * dy * map_[(y+1)*size_.x + x];
	height += dx * dy * map_[(y+1)*size_.x + x+1];
	return height;
}

const glm::vec3 &Terrain::normal_at(int x, int y) const {
	return vertices_[y*size_.x + x].normal;
}

glm::vec3 Terrain::normal_at(float x_, float y_) const {
	if(x_ > size_.x * horizontal_scale_|| x_ < 0 || y_ > size_.y*horizontal_scale_ || y_ < 0)
		return glm::vec3(0, 1, 0);
	int x = (int) (x_/horizontal_scale_);
	int y = (int) (y_/horizontal_scale_);
	float dx = (x_/horizontal_scale_) - x;
	float dy = (y_/horizontal_scale_) - y;
	glm::vec3 normal(0.f);
	normal += (1.f-dx) * (1.f-dy) * normal_at(x,y);
	normal += dx * (1.f-dy) * normal_at(y,x+1);
	normal += (1.f-dx) * dy * normal_at(y+1,x);
	normal += dx * dy * normal_at(y+1, x+1);
	return normal;
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

	shader_->bind();

	Shader::upload_model_matrix(matrix());

	Shader::upload_material(material);

	data_texture_->texture_bind(Shader::TEXTURE_2D_0);
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
