#ifndef TERRAIN_H
#define TERRAIN_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <SDL/SDL.h>

#include "shader.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "texture.hpp"

class Terrain : public Mesh {
	float horizontal_scale_;
	float vertical_scale_;
	SDL_Surface * data_map_;
	Texture2D * data_texture_; 
	Shader * shader_;
	glm::ivec2 size_;
	float * map_;

	void generate_terrain();

	float height_from_color(const glm::vec4 &color) const ;

	TextureArray * textures_[2];

	float height_at(int x, int y) const;
	const glm::vec3 &normal_at(int x, int y) const;

	public:
		float vertical_scale() { return vertical_scale_; };
		Terrain(const std::string &file, float horizontal_scale, float vertical_scale, TextureArray * color_, TextureArray * normal_);
		virtual ~Terrain();
		virtual void render();
		const glm::ivec2 &size() const;
		static glm::vec4 get_pixel_color(int x, int y, SDL_Surface * surface, const glm::ivec2 &size);

		float height_at(float x, float y) const;
		glm::vec3 normal_at(float x, float y) const;

		Material material;

		/*
		 * Once this has been called get_pixel_color can not be called
		 */
		void free_surface();
};

#endif
