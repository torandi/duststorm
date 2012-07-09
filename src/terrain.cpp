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
}

Terrain::Terrain(const std::string &name, float horizontal_scale, float vertical_scale, Texture2D * blendmap, TextureArray * color_, TextureArray * normal_) :
		horizontal_scale_(horizontal_scale),
		vertical_scale_(vertical_scale),
		map_(nullptr),
		terrain_map_(blendmap),
		base_(name)
		{
	textures_[0] = color_;
	textures_[1] = normal_;
	shader_ = Shader::create_shader("terrain");

	glm::ivec2 size;
	heightmap_ = TextureBase::load_image(base_ + "_map.png", &size);
	height_texture_ = Texture2D::from_filename(base_ + "_map.png");
	width_ = size.x;
	height_ = size.y;
	generate_terrain();	
	SDL_FreeSurface(heightmap_);

	absolute_move(-glm::vec3(width_*horizontal_scale_, 0, height_*horizontal_scale_)/2.0f);
}

Texture2D * Terrain::heightmap() const {
	return height_texture_;
}
Texture2D * Terrain::blendmap() const { return terrain_map_; }

void Terrain::generate_terrain() {
	unsigned long numVertices = width_*height_;

	map_ = new float[numVertices];

	fprintf(verbose,"Generating terrain...\n");
	fprintf(verbose,"World size: %dx%d, scale: %fx%f\n", width_, height_, horizontal_scale_, vertical_scale_);

	vertices_ = std::vector<vertex_t>(numVertices);
	for(int y=0; y<height_; ++y) {
		for(int x=0; x<width_; ++x) {
			vertex_t v;
			int i = y * width_ + x;
			glm::vec4 color = get_pixel_color(x, y);
			float h = height_from_color(color);
			v.position = glm::vec3(horizontal_scale_*x, h*vertical_scale_, horizontal_scale_*y); 
			v.tex_coord = glm::vec2(1.f-v.position.x/(width_*horizontal_scale_), 1.f-v.position.z/(height_*horizontal_scale_));
			vertices_[i] = v;
			map_[i] =  h*vertical_scale_;
		}
	}
	unsigned long indexCount = (height_ - 1 ) * (width_ -1) * 6;

	//build indices
	indices_ = std::vector<unsigned int>(indexCount);
	for(int x=0; x<width_- 1; ++x) {
		for(int y=0; y<height_- 1; ++y) {
			int i = y * (width_-1) + x;
			indices_[i*6 + 2] = (x + 1) + y*width_;
			indices_[i*6 + 1] = x + (y+1)*width_;
			indices_[i*6 + 0] = x + y*width_;

			indices_[i*6 + 5] = (x + 1) + y*width_;
			indices_[i*6 + 4] = (x+1) + (y+1)*width_;
			indices_[i*6 + 3] = x + (y+1)*width_;
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

float Terrain::height_from_color(const glm::vec4 &color) {
	return color.r;
}

float Terrain::get_height_at(int x, int y) {
	return map_[y*width_ + x];
}

float Terrain::get_height_at(float x_, float y_) {
	int x = (int) (x_/horizontal_scale_);
	int y = (int) (y_/horizontal_scale_);
	float dx = (x_/horizontal_scale_) - x;
	float dy = (y_/horizontal_scale_) - y;
	float height=0;
	height += (1.0-dx) * (1.0-dy) * map_[y*width_ + x];
	height += dx * (1.0-dy) * map_[y*width_ + x+1];
	height += (1.0-dx) * dy * map_[(y+1)*width_ + x];
	height += dx * dy * map_[(y+1)*width_ + x+1];
	return height;
}

glm::vec4 Terrain::get_pixel_color(int x, int y) {
	glm::vec4 color;

	Uint32 temp, pixel;
	Uint8 red, green, blue, alpha;
	pixel = ((Uint32*)heightmap_->pixels)[(height_-(y+1))*(width_)+(width_-(x+1))];

	SDL_PixelFormat * fmt=heightmap_->format;

	/* Get Red component */
	temp = pixel & fmt->Rmask;  /* Isolate red component */
	temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
	red = (Uint8)temp;

	/* Get Green component */
	temp = pixel & fmt->Gmask;  /* Isolate green component */
	temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
	green = (Uint8)temp;

	/* Get Blue component */
	temp = pixel & fmt->Bmask;  /* Isolate blue component */
	temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
	blue = (Uint8)temp;

	/* Get Alpha component */
	temp = pixel & fmt->Amask;  /* Isolate alpha component */
	temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
	temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
	alpha = (Uint8)temp;

	color.r = (float)red/0xFF;
	color.g = (float)green/0xFF;
	color.b = (float)blue/0xFF;
	color.a = (float)alpha/0xFF;

	return color;	
}

void Terrain::render() {
	Shader::upload_model_matrix(matrix());

	terrain_map_->texture_bind(Shader::TEXTURE_2D_0);
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
