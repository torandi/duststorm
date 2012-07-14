#include "area.hpp"
#include "data.hpp"
#include "utils.hpp"

#include "game.hpp"

#define ITEM_SPAWN_AREA 2.f

static const float marker_size = 0.5f;

Area::Area(const std::string &name, Game &game_) : game(game_), name_(name) {
	char * str;
	if(asprintf(&str, PATH_BASE "/game/areas/%s.yaml", name.c_str()) == -1) abort();

	YAML::Node config = YAML::LoadFile(str);
	free(str);

	terrain_shader = Shader::create_shader(config["shader"].as<std::string>("terrain"));
	u_fog_density = terrain_shader->uniform_location("fog_density");
	u_marker = terrain_shader->uniform_location("marker_position");

	required_blood = config["required_blood"].as<int>();


	//Create terrain
	terrain_textures[0] = TextureArray::from_filename(config["textures"][0].as<std::string>().c_str(),config["textures"][1].as<std::string>().c_str(), nullptr);
	terrain_textures[0]->texture_bind(Shader::TEXTURE_ARRAY_0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//terrain_textures[1] = TextureArray::from_filename("dirt_normal.png","grass_normal.png", nullptr);
	terrain_textures[1] = TextureArray::from_filename("default_normalmap.png","default_normalmap.png", nullptr); //TODO
	terrain_textures[1]->texture_bind(Shader::TEXTURE_ARRAY_0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	float hscale = config["scale"].as<float>(1.f);
	terrain = new Terrain(name, hscale, config["height"].as<float>(), terrain_textures[0], terrain_textures[1]);

	terrain_shader->bind();
	glUniform1f(terrain_shader->uniform_location("marker_size"), marker_size/terrain->size().x);

	fog_density = config["fog"].as<float>(0.02f);

	int light_count = 1;

	YAML::Node lights_node = config["lights"];
	if(lights_node.IsSequence()) {
		for(const YAML::Node &n : lights_node) {
			if(light_count == MAX_NUM_LIGHTS) {
				fprintf(stderr, "Warning! To many lights in area %s, only %d allowed\n", name.c_str(), MAX_NUM_LIGHTS);
				break;
			}
			light_height[light_count-1] = n["height"].as<float>(1.f);
			lights.lights[light_count].type = (Light::light_type_t) n["type"].as<int>(Light::POINT_LIGHT);
			lights.lights[light_count].intensity = n["color"].as<glm::vec3>();
			glm::vec2 pos = n["position"].as<glm::vec2>();
			move_light(light_count-1, pos);
			++light_count;
		}
	}

	fprintf(verbose, "Created %d lights in area %s\n", light_count, name.c_str());

	lights.ambient_intensity() = config["ambient_intensity"].as<glm::vec3>(glm::vec3(0.f));
	lights.num_lights() = light_count;
	lights.lights[0].set_position(game.player->position());
	lights.lights[0].intensity = game.player->light_color;
	lights.lights[0].type = Light::POINT_LIGHT;
	//lights.lights[1].constant_attenuation = 0.f;
	//lights.lights[0].linear_attenuation = 0.01f;
	lights.lights[0].quadratic_attenuation = 0.06f;

	game.player->add_position_callback(&(lights.lights[0]), game.player->light_offset);

	skycolor = config["skycolor"].as<Color>(Color::red);

	//load entry points:
	YAML::Node ep_node = config["entry_points"];
	for(auto it = ep_node.begin(); it != ep_node.end(); ++it) {
		std::string e_name = it->first.as<std::string>();
		glm::vec2 pos = it->second.as<glm::vec2>();
		entry_points[e_name] = pos;
	}

	//Add objects:
	YAML::Node obj_node = config["objects"];
	for(const YAML::Node &n : obj_node) {
		ObjectTemplate * obj = game.create_object(n["type"].as<std::string>(), n, this);
		objects.push_back(obj);
	}
	

	u_highlight = shaders[SHADER_NORMAL]->uniform_location("highlight");
	shaders[SHADER_NORMAL]->bind();
	glUniform3f(u_highlight, 0.f, 0.f, 0.f);
	Shader::unbind();

	float wall_scale = config["walls"]["scale"].as<float>(1.f);
	glm::vec2 texture_scale = config["walls"]["texture_scale"].as<glm::vec2>(glm::vec2(1.f));
	wall_material.texture = Texture2D::from_filename(config["walls"]["texture"].as<std::string>());
	wall_material.normal_map = Texture2D::from_filename(config["walls"]["normal_map"].as<std::string>("default_normalmap.png"));

	//Create walls:
	std::vector<Mesh::vertex_t> vertices;
	std::vector<unsigned int> indices;
	for(int y=0; y < terrain->size().y; ++y) {
		for(int x=0; x < terrain->size().x; ++x) {
			float w = terrain->get_wall_at(x, y)*wall_scale;
			glm::vec2 uv_x( fmod(x*texture_scale.x, 1.f), 0.f);
			glm::vec2 uv_y(0.f, fmod(y*texture_scale.y, 1.f));
			if( w > 0.1f ) {
				Mesh::vertex_t v;
				int index = vertices.size();

				if(terrain->get_wall_at(x, y-1) <= 0.1f) {
					//Face

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y), y*hscale);
					v.tex_coord = uv_x + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_x + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y), y*hscale);
					v.tex_coord = uv_x + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);


					//Face

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y)+w, y*hscale);
					v.tex_coord = uv_x + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y), y*hscale);
					v.tex_coord = uv_x + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_x + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

				}

				if(terrain->get_wall_at(x, y+1) <= 0.1f) {
					//Face


					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y+1), (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, (y+1))+w, (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, (y+1)), (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);
					//Face


					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, (y+1))+w, (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, (y+1)), (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(1, 0)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, (y+1))+w, (y+1)*hscale);
					v.tex_coord = uv_x + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

				}

				if(terrain->get_wall_at(x-1, y) <= 0.1f) {
					//Face
					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y), y*hscale);
					v.tex_coord = uv_y + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					//Face
					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1)+w, (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);
				}

				if(terrain->get_wall_at(x+1, y) <= 0.1f) {
					//Face

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y), y*hscale);
					v.tex_coord = uv_y + glm::vec2(1, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					//Face
					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y+1)+w, (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(0, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y+1), (y+1)*hscale);
					v.tex_coord = uv_y + glm::vec2(0, 0.f)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

					v.position = glm::vec3((x+1)*hscale, terrain->get_height_at((x+1), y)+w, y*hscale);
					v.tex_coord = uv_y + glm::vec2(1, wall_scale)*texture_scale;
					vertices.push_back(v);
					indices.push_back(index++);

				}

				//Face

				v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y)+w, y*hscale);
				v.tex_coord = uv_x + uv_y + glm::vec2(1, 0.f)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1)+w, (y+1)*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(1, 1)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y)+w, y*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(0, 0.f)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);


				//Face

				v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y+1)+w, (y+1)*hscale);
				v.tex_coord = uv_x + uv_y + glm::vec2(0, 1)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3((x+1)*hscale, terrain->get_height_at(x+1, y)+w, y*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(0, 0.f)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);

				v.position = glm::vec3(x*hscale, terrain->get_height_at(x, y+1)+w, (y+1)*hscale);
				v.tex_coord = uv_x +  uv_y +glm::vec2(1, 1)*texture_scale;
				vertices.push_back(v);
				indices.push_back(index++);
			}
		}
	}

	if(indices.size() > 0) {
		wall = new Mesh(vertices, indices);
		wall->generate_normals();
		wall->generate_tangents_and_bitangents();
		wall->ortonormalize_tangent_space();
		wall->generate_vbos();
	} else {
		wall = nullptr;
	}
	//Spawn enemies:
	const YAML::Node &e_node = config["enemies"];
	for(auto e : e_node) {
		for(int i=0;i<e["count"].as<int>(1); ++i) {
			glm::ivec2 pos = terrain->spawnmap[rand()%terrain->spawnmap.size()];
			enemies.push_back(game.create_enemy(e, glm::vec2(pos.x*hscale, pos.y*hscale), this));
			printf("Spawn %s at %d %d\n", e["name"].as<std::string>().c_str(), pos.x, pos.y);
		}
	}

	splattermap = new RenderTarget(glm::ivec2(1024, 1024), GL_RGB8, false);
	splattermap->bind();
	splattermap->clear(Color::black);
	splattermap->unbind();

	blood = Texture2D::from_filename("blood.png");
	blood_quad = new Quad();
	blood_quad->set_scale(32.f);
}

Area::~Area() {
	delete terrain;

	delete terrain_textures[0];
	delete terrain_textures[1];
}

const glm::vec2 &Area::get_entry_point(const std::string &name) {
	auto it = entry_points.find(name);
	if(it == entry_points.end()) {
		printf("Unknown entry point '%s' in area %s\n", name.c_str(), name_.c_str());
		abort();
	}
	return it->second;
}

void Area::move_light(int id, const glm::vec2 &new_pos) {
	lights.lights[id+1].set_position(glm::vec3(new_pos.x, height_at(new_pos) + light_height[id], new_pos.y));
}

void Area::upload_lights() {
	Shader::upload_lights(lights);
}

void Area::update(float dt) {
	objects.remove_if([](ObjectTemplate * o) {
		if(o->destroyed) {
			delete o;
			return true;
		} else {
			return false;
		}
	});

	enemies.remove_if([](const Enemy * e) {
		if(e->dead) {
			delete e;
			return true;
		} else {
			return false;
		}
	});

	for(auto it : objects) {
		it->update(dt);
	}

	for(auto it : enemies) {
		it->update(dt);
	}
}

bool Area::mouse_at(const glm::vec2 &pos) {
	bool on = false;
	for(auto it : objects) {
		if(it->obj->hit(pos, game.player->click_radius, true)) {
			it->highlighted = true;
			on = true;
		} else {
			it->highlighted = false;
		}
	}

	for(auto it : enemies) {
		if(it->hit(pos, game.player->click_radius, true)) {
			it->highlighted = true;
			on = true;
		} else {
			it->highlighted = false;
		}
	}
	return on;
}

bool Area::click_at(const glm::vec2 &pos, int btn) {
	if(btn == 1) {
		bool click = false;
		for(auto it : objects) {
			if(it->obj->hit(pos, game.player->click_radius, true)) {
				it->click();
				click = true;
			}
		}
		for(auto it : enemies) {
			if(it->hit(pos, game.player->click_radius, true)) {
				game.player->swing();
				return true;
			}
		}
		return click;
	}
	return false;
}

void Area::attack(int id) {
	for(auto it : objects) {
		if(it->obj->hit(game.player->center(), game.player->weapon_radius[id])) {
			it->hit();
		}
	}

	int count = 0;
	for(auto it : enemies) {
		if(it->hit(game.player->center(), game.player->radius + game.player->weapon_radius[id])) {
			it->life -= game.player->weapon_damage[id];
			if(it->life <= 0) {
				it->dead = true;
				spawn_splatter(it->center());
				++count;
				for(Enemy::drop_t &d : it->drops) {
					 spawn_pickups( d.name, it->center(), d.rate.x + (rand() % (d.rate.y+1)));
				}
			}
		}
	}
	if(count > 0) {
		game.play_sfx("splatt");
		game.player->score += 100*count;
	}

	if(count >= 3) {
		game.player->score += 100*count*2;
		game.play_sfx("multikill", 0.5f);
	} else if(count == 2) {
		game.player->score += 100*count;
		//Double kill
		game.play_sfx("double_kill", 0.5f);
	}
}


void Area::spawn_splatter(const glm::vec2 &pos) {
	ObjectTemplate * ot = Decoration::create("splatt", game);
	ot->obj->height = 1.f;
	ot->obj->move_to(pos);
	ot->ttl = 4.0;
	ot->obj->update(0.1f);
	ParticlesVFX * pv = (ParticlesVFX*) ot->obj;
	pv->set_spawn_rate(ot->obj->vfx_state, 0.f, 0.f);
	objects.push_back(ot);

	splattermap->bind();
	blood->texture_bind(Shader::TEXTURE_2D_0);
	blood_quad->set_position(glm::vec3(pos.x/terrain->size().x, pos.y/terrain->size().y, 0));
	blood_quad->render();
	splattermap->unbind();
}

void Area::render(const glm::vec2 &marker_position) {
	RenderTarget::clear(skycolor);
	terrain_shader->bind();
	glUniform1f(u_fog_density, fog_density);
	glUniform2f(u_marker, (marker_position.x/terrain->size().x), 1.f - (marker_position.y/terrain->size().y));
	splattermap->texture_bind(Shader::TEXTURE_2D_2);
	terrain->render();

	if(wall != nullptr) {
		shaders[SHADER_NORMAL]->bind();
		wall_material.activate();
		wall->render();
		wall_material.deactivate();
	}

	game.player->render();

	for(auto it : enemies) {
		shaders[SHADER_NORMAL]->bind();
		if(it->highlighted) {
			glUniform3f(u_highlight, 0.f, 0.f, 0.8f);
		} else
			glUniform3f(u_highlight, 0.f, 0.f, 0.f);
		it->render();
	}

	for(auto it : objects) {
		shaders[SHADER_NORMAL]->bind();
		if(it->highlighted) 
			glUniform3f(u_highlight, 0.f, 0.f, 0.8f);
		else
			glUniform3f(u_highlight, 0.f, 0.f, 0.f);
		it->render();
	}
	shaders[SHADER_NORMAL]->bind();
	glUniform3f(u_highlight, 0.f, 0.f, 0.f);

}

float Area::height_at(const glm::vec2 &pos) const {
	return terrain->get_height_at(pos.x, pos.y);
}

bool Area::collision_at(const glm::vec2 &pos, Object2D * o) const {
	if(!terrain->get_collision_at(pos.x, pos.y)) {
		if(o->global_id == game.player->global_id) {
			for(auto it : objects) {
				if(it->obj->hit(pos, game.player->radius)) {
					if(it->collision())
						return true;
				}
			}
		}
		for(auto it : enemies) {
			if(it->global_id != o->global_id && it->hit(pos, game.player->radius/4.f)) {
					return true;
			}
		}
		return false;
	} else {
		return true;
	}
}

void Area::spawn_pickups(const std::string &name, const glm::vec2 &center, int count) {
	for(int c = 0; c < count; ++c) {
		glm::vec2 pos = center + glm::vec2( ((2.f*frand())-1.f)*ITEM_SPAWN_AREA,  ((2.f*frand())-1.f)*ITEM_SPAWN_AREA );
		objects.push_back(game.spawn_pickup(name, pos));
	}
}
