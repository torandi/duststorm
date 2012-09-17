#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <string>
#include <GL/glew.h>

#include "camera.hpp"

class Skybox {
	public:
		Skybox(std::string skybox_path);
		~Skybox();

		TextureCubemap * texture;

		void render(const Camera &camera) const;

	private:
		static Shader* shader;
		static GLuint vbo;
		static const float vertices[2*3*36];
		static const char * texture_names[];
};

#endif
