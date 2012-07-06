#ifndef TERRAIN_H
#define TERRAIN_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <SDL/SDL.h>

#include "shader.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "movable_object.hpp"

class Terrain : Mesh {
	float horizontal_scale_;
	float vertical_scale_;
	SDL_Surface * heightmap_;
	int width_, height_;
	Texture2D * texture_map_;

	void generate_terrain();

	glm::vec4 get_pixel_color(int x, int y);
	float height_from_color(const glm::vec4 &color);

	const float texture_scale_;
	Texture2D * terrain_map_;
	TextureArray * textures_[2];

	std::string base_;
	Shader * shader_;
	public:
		float height() { return height_; };
		float width() { return width_; };
		float vertical_scale() { return vertical_scale_; };
		Terrain(const std::string &name, float horizontal_scale, float vertical_scale, TextureArray * color_, TextureArray * normal_);
		virtual ~Terrain();
		virtual void render();
};

#endif
