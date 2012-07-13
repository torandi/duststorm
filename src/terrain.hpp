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
	SDL_Surface *heightmap_, *data_map_;
	glm::ivec2 hm_size, dm_size;
	float * map_;
	float * wall_;
	enum {
		DATA_FALSE = 0,
		DATA_TRUE = 1,
		DATA_UNDEFINED = 2
	};
	unsigned char * data_;

	void generate_terrain();

	float height_from_color(const glm::vec4 &color) const ;

	Texture2D * terrain_data_, *height_texture_;
	TextureArray * textures_[2];

	std::string base_;
	Shader * shader_;
	public:
		static glm::vec4 get_pixel_color(int x, int y, SDL_Surface * surface, const glm::ivec2 &size);
		float vertical_scale() { return vertical_scale_; };
		Texture2D * heightmap() const;
		Texture2D * datamap() const;
		Terrain(const std::string &name, float horizontal_scale, float vertical_scale, TextureArray * color_, TextureArray * normal_);
		virtual ~Terrain();
		virtual void render();
		const glm::ivec2 &size() const;

	float get_height_at(int x, int y) const;
	float get_height_at(float x, float y) const;
	bool get_collision_at(float x, float y) const;
	bool get_collision_at(int x, int y) const;
	float get_wall_at(int x, int y) const;

};

#endif
