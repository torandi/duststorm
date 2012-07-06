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

Terrain::~Terrain() { }

Terrain::Terrain(const std::string &name, float horizontal_scale, float vertical_scale, TextureArray * color_, TextureArray * normal_) :
		horizontal_scale_(horizontal_scale),
		vertical_scale_(vertical_scale),
		texture_scale_(128.0f) ,
		base_(name)
		{
	textures_[0] = color_;
	textures_[1] = normal_;
	shader_ = Shader::create_shader("terrain");

	heightmap_ = load_image();
	terrain_map_ = Texture2D::from_filename(base_+"_map.png");
	generate_terrain();	
	SDL_FreeSurface(heightmap_);

	//position_-=glm::vec3(width_*horizontal_scale_, 0, height_*horizontal_scale_)/2.0f;
		}

void Terrain::generate_terrain() {
	unsigned long numVertices = width_*height_;

	fprintf(verbose,"Generating terrain...\n");
	fprintf(verbose,"World size: %dx%d\n", width_, height_);

	vertices_ = std::vector<vertex_t>(numVertices);
	for(int y=0; y<height_; ++y) {
		for(int x=0; x<width_; ++x) {
			vertex_t v;
			int i = y * width_ + x;
			glm::vec4 color = get_pixel_color(x, y);
			float h = height_from_color(color);
			v.position = glm::vec3(horizontal_scale_*x, h*vertical_scale_, horizontal_scale_*y); 
			v.tex_coord = glm::vec2(v.position.x/texture_scale_, v.position.z/texture_scale_);
			vertices_[i] = v;
		}
	}
	unsigned long indexCount = (height_ - 1 ) * (width_ -1) * 6;

	//build indices
	indices_ = std::vector<unsigned int>(indexCount);
	for(int x=0; x<width_- 1; ++x) {
		for(int y=0; y<height_- 1; ++y) {
			int i = y * (width_-1) + x;
			indices_[i*6 + 0] = x + y*width_;
			indices_[i*6 + 1] = x + (y+1)*width_;
			indices_[i*6 + 2] = (x + 1) + y*width_;
			indices_[i*6 + 3] = x + (y+1)*width_;
			indices_[i*6 + 4] = (x+1) + (y+1)*width_;
			indices_[i*6 + 5] = (x + 1) + y*width_;
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
//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

float Terrain::height_from_color(const glm::vec4 &color) {
	return color.r;
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

SDL_Surface * Terrain::load_image() {
	/* Load image using SDL Image */
	std::string heightmap = PATH_BASE "textures/" +  base_ + "_map.png";
	SDL_Surface* surface = IMG_Load(heightmap.c_str());
	if ( !surface ){
	  fprintf(stderr, "Failed to load heightmap at %s\n", heightmap.c_str());
		abort();
	}
	width_ = surface->w;
	height_ = surface->h;
	SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			width_, height_,
			32,
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
	);

	if ( !rgba_surface ) {
	  fprintf(stderr, "Failed to create RGBA surface\n");
		abort();
	}

	/* Save the alpha blending attributes */
	Uint32 saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
	Uint8 saved_alpha = surface->format->alpha;
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, 0, 0);
	}

	SDL_Rect srcrect;
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = width_;
	srcrect.h = height_;

	SDL_BlitSurface(surface, &srcrect, rgba_surface, 0);

	/* Restore the alpha blending attributes */
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, saved_flags, saved_alpha);
	}


	SDL_FreeSurface(surface);
	return rgba_surface;
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
