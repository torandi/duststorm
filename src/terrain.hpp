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

class Terrain : public Mesh {
	float horizontal_scale_;
	float vertical_scale_;
	SDL_Surface * heightmap_;
	int width_, height_;
	float * map_;
	Texture2D * texture_map_;

	void generate_terrain();

	glm::vec4 get_pixel_color(int x, int y);
	float height_from_color(const glm::vec4 &color);

	Texture2D * terrain_map_, *height_texture_;
	TextureArray * textures_[2];

	std::string base_;
	Shader * shader_;
	public:
		float height() { return height_; };
		float width() { return width_; };
		float vertical_scale() { return vertical_scale_; };
		Texture2D * heightmap() const;
		Texture2D * blendmap() const;
		Terrain(const std::string &name, float horizontal_scale, float vertical_scale, Texture2D * blendmap, TextureArray * color_, TextureArray * normal_);
		virtual ~Terrain();
		virtual void render();

	float get_height_at(int x, int y);
	float get_height_at(float x, float y);

};

#endif
