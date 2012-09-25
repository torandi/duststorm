#include "text.hpp"
#include "mesh.hpp"
#include "quad.hpp"
#include "shader.hpp"
#include "globals.hpp"

#include <vector>
#include <string>
#include <glm/glm.hpp>

Text::Text() : Mesh() {
	shader = Shader::create_shader("text");
	u_font_offset = shader->uniform_location("font_offset");
	texture = Texture2D::from_filename(PATH_BASE "/data/textures/number_fonts.png");
}

Text::~Text() {
	for(Quad * q : quads) {
		delete q;
	}
}

void Text::set_number(int nr) {
	char buff[256];
	sprintf(buff, "%d", nr);
	set_text(std::string(buff));
}

void Text::set_text(const std::string &str) {
	if(str.length() > quads.size()) {
		int add = (str.length() - quads.size());
		for(int i=0; i<add; ++i) {
			add_quad();
		}
	}

	offsets.clear();

	for(char c : str) {
		if(c < 0x30 || c > 0x39) {
			printf("Text contains non-numeric char\n");
			abort();
		}
		offsets.push_back(glm::vec2( (c-0x30) * 0.1f, 0.f));
	}
}

void Text::render_geometry(const glm::mat4& m) {
	shader->bind();
	texture->texture_bind(Shader::TEXTURE_2D_0);
	for(int i = 0; i < offsets.size(); ++i) {
		glUniform2f(u_font_offset, offsets[i].x, offsets[i].y);
		quads[i]->render(matrix());
	}
}

void Text::add_quad() {
	Quad * q = new Quad(glm::vec2(0.1f, -1.f));
	q->set_position(glm::vec3(quads.size(), 0.f, 0.f));
	quads.push_back(q);
}
