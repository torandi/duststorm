#ifndef TEXT_HPP
#define TEXT_HPP

#include "mesh.hpp"
#include "quad.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>

class Text : public Mesh {
	public:
		Text();
		virtual ~Text();

		void set_text(const std::string &str);
		void set_number(int nr);

		virtual void render_geometry(const glm::mat4& m = glm::mat4());
	private:
		GLuint u_font_offset;
		Shader * shader;
		Texture2D * texture;
		std::vector<Quad*> quads;
		void add_quad();

		std::vector<glm::vec2> offsets;
};

#endif
