#ifndef AREA_HPP
#define AREA_HPP

#include "shader.hpp"
#include "terrain.hpp"
#include "texture.hpp"
#include "lights_data.hpp"
#include "color.hpp"
#include "yaml-helper.hpp"
#include "material.hpp"

#include <glm/glm.hpp>
#include <string>

#include <list>
#include <map>

class Game;

class Area {
	public:
		Area(const std::string &name, Game &game_);
		~Area();
		
		float height_at(const glm::vec2 &pos) const;
		bool collision_at(const glm::vec2 &pos) const; //This one may also trigger closness based stuff

		bool click_at(const glm::vec2 &pos); //return true if this clicks something
		bool mouse_at(const glm::vec2 &pos); //return true if this marks something in the area

		void update(float dt);
		void render(const glm::vec2 &marker_position);

		void upload_lights();

		//Object at

		void move_light(int id, const glm::vec2 &new_pos);

		const glm::vec2 &get_entry_point(const std::string &name);
	private:
		Game &game;
		std::string name_;	
		//Entity * highlighted;

		Shader * terrain_shader;
		Terrain * terrain;
		Texture2D * terrain_datamap;
		Material wall_material;
		TextureArray * terrain_textures[2];

		Mesh * wall;

		LightsData lights;

		Color skycolor;

		GLint u_fog_density, u_marker;
		float fog_density;
		float light_height[MAX_NUM_LIGHTS-1];

		GLint u_highlight;

		std::map<std::string, glm::vec2> entry_points;

		//TODO: More properties
		// * links to other locations
		// * objects [maybe global list of templates]
		// * enemies [maybe global list of templates]
};

#endif
