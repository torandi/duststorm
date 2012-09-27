#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "text.hpp"
#include "mesh.hpp"
#include "quad.hpp"
#include "shader.hpp"
#include "globals.hpp"
#include "color.hpp"

#include <vector>
#include <string>
#include <glm/glm.hpp>

Text::Text(Text::alignment_t alignment_) : Mesh(), color(Color::white), alignment(alignment_) {
	shader = Shader::create_shader("text");
	u_font_offset = shader->uniform_location("font_offset");
	u_color = shader->uniform_location("color");
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

void Text::set_alignment(Text::alignment_t alignment_) {
	alignment = alignment_;
	quads.clear();
	for(unsigned int i=0; i<offsets.size(); ++i) {
		add_quad();
	}
}

void Text::render_geometry(const glm::mat4& m) {
	shader->bind();
	texture->texture_bind(Shader::TEXTURE_2D_0);
	glUniform4f(u_color, color.r, color.g, color.b, color.a);
	for(unsigned int i = 0; i < offsets.size(); ++i) {
		int index = (alignment == LEFT_ALIGNED) ? i : (offsets.size() - (i + 1));
		glUniform2f(u_font_offset, offsets[index].x, offsets[index].y);
		quads[i]->render(matrix());
	}
}

void Text::add_quad() {
	Quad * q = new Quad(glm::vec2(0.1f, -1.f));
	q->set_position(glm::vec3((alignment == LEFT_ALIGNED ? 1.0 : -1.0) * (float)quads.size(), 0.f, 0.f));
	quads.push_back(q);
}

void Text::set_color(const Color &c) {
	color = c;
}
