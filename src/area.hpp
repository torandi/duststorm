#ifndef AREA_HPP
#define AREA_HPP

#include "shader.hpp"
#include "terrain.hpp"
#include "texture.hpp"
#include "lights_data.hpp"
#include "color.hpp"
#include "yaml-helper.hpp"

#include <glm/glm.hpp>
#include <string>

#include <list>

class Game;

class Area {
	public:
		Area(const std::string &name, Game &game_);
		~Area();
		
		float height_at(const glm::vec2 &pos) const;
		bool collision_at(const glm::vec2 &pos) const;

		bool click_at(const glm::vec2 &pos);
		bool mouse_at(const glm::vec2 &pos); //return true if this marks something in the area

		void update(float dt);
		void render();

		void upload_lights();

	private:
		Game &game;

		Shader * terrain_shader;
		Terrain * terrain;
		Texture2D * terrain_datamap;
		TextureArray * terrain_textures[2];

		LightsData lights;

		Color skycolor;

		GLint u_fog_density;
		float fog_density;

		GLint u_highlight;

		//TODO: More properties
		// * links to other locations
		// * entry points
		// * objects [maybe global list of templates]
		// * enemies [maybe global list of templates]
};

#endif
