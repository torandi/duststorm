#ifndef GAME_CPP
#define GAME_CPP

#include <SDL/SDL.h>
#include "globals.hpp"
#include "camera.hpp"
#include "rendertarget.hpp"
#include "shader.hpp"
#include "quad.hpp"
#include "terrain.hpp"
#include "color.hpp"
#include "lights_data.hpp"

static const Color skycolor = Color::rgb(149.0f / 255.0f, 178.0f / 255.0f, 178.0f / 255.0f);


class Game {
	public:
		Game() : camera(75.f, resolution.x/(float)resolution.y, 0.1f, 100.f) {

			camera.set_position(glm::vec3(35.750710, 17.926385, 6.305542));
			camera.look_at(glm::vec3(35.750710, 17.926385, 7.305542));

			lights.ambient_intensity() = glm::vec3(0.0f);
			lights.num_lights() = 1;
			lights.lights[0].set_position(glm::vec3(10, 50.f, 10.f));
			lights.lights[0].intensity = glm::vec3(0.8f);
			lights.lights[0].type = Light::POINT_LIGHT;
			lights.lights[0].quadratic_attenuation = 0.00002f;

			terrain_shader = Shader::create_shader("terrain");

			//Create terrain
			terrain_textures[0] = TextureArray::from_filename("dirt.png","grass.png", nullptr);
			terrain_textures[0]->texture_bind(Shader::TEXTURE_ARRAY_0);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

			//terrain_textures[1] = TextureArray::from_filename("dirt_normal.png","grass_normal.png", nullptr);
			terrain_textures[1] = TextureArray::from_filename("default_normalmap.jpg","default_normalmap.jpg", nullptr);
			terrain_textures[1]->texture_bind(Shader::TEXTURE_ARRAY_0);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

			terrain_blendmap = Texture2D::from_filename("park_blend.png");
			terrain = new Terrain("park", 1.f, 20.f, terrain_blendmap, terrain_textures[0], terrain_textures[1]);

			composition = new RenderTarget(resolution, GL_RGB8, false);
			screen = new RenderTarget(resolution, GL_RGB8, false);
			downsample[0] = new RenderTarget(glm::ivec2(200, 200), GL_RGB8, false, GL_LINEAR);
			downsample[1] = new RenderTarget(glm::ivec2(100, 100), GL_RGB8, false, GL_LINEAR);
			downsample[2] = new RenderTarget(glm::ivec2( 50, 50), GL_RGB8, false, GL_LINEAR);
		}

		~Game() {
			delete terrain;

			delete terrain_textures[0];
			delete terrain_textures[1];
			delete terrain_blendmap;

			delete composition;
			for(RenderTarget * ds: downsample) {
				delete ds;
			}
		}

		void update(float dt) {

		}

		void input(const SDL_Event &event) {

		}

		void render_composition(){
			RenderTarget::clear(Color::black);

			Shader::upload_state(resolution);
			Shader::upload_projection_view_matrices(composition->ortho(), glm::mat4());
			glViewport(0, 0, resolution.x, resolution.y);

			//downsample[1]->texture_bind(Shader::TEXTURE_2D_2);
			composition->draw(shaders[SHADER_PASSTHRU]);
		}

		void render() {
			composition->bind();

			RenderTarget::clear(skycolor);
			glDisable(GL_CULL_FACE);

			terrain_shader->bind();
			Shader::upload_lights(lights);
			Shader::upload_camera(camera);
			terrain->render();

			Shader::unbind();
			composition->unbind();

			//Blur
			
			/*RenderTarget* prev = composition;
			for ( int i = 0; i < 2; i++ ){
				Shader::upload_state(downsample[i]->texture_size());
				Shader::upload_projection_view_matrices(downsample[i]->ortho(), glm::mat4());
				downsample[i]->with([prev,i](){
						prev->draw(shaders[SHADER_BLUR], glm::ivec2(0,0), downsample[i]->texture_size());
					});
				prev = downsample[i];
			}*/
			
			screen->with([this]() {
					this->render_composition();
				});

			//Output
			RenderTarget::clear(Color::magenta);
			Shader::upload_projection_view_matrices(screen_ortho, glm::mat4());
			screen->draw(shaders[SHADER_PASSTHRU]);
		}
	private:
		Camera camera;
		RenderTarget *screen, *composition, *downsample[3];
		LightsData lights;

		Shader * terrain_shader;
		Terrain * terrain;
		Texture2D * terrain_blendmap;
		TextureArray * terrain_textures[2];
};

#endif
