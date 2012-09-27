#ifndef TEXT_HPP
#define TEXT_HPP

#include "mesh.hpp"
#include "quad.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "color.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>

class Text : public Mesh {
	public:
		enum alignment_t {
			LEFT_ALIGNED,
			RIGHT_ALIGNED
		};

		Text(alignment_t alignment_ = LEFT_ALIGNED);
		virtual ~Text();

		void set_alignment(alignment_t alignment_);

		void set_text(const std::string &str);
		void set_number(int nr);

		void set_color(const Color &c);

		virtual void render_geometry(const glm::mat4& m = glm::mat4());

	private:
		void add_quad();

		GLuint u_font_offset, u_color;
		Shader * shader;
		Texture2D * texture;
		Color color;
		std::vector<Quad*> quads;
		alignment_t alignment;

		std::vector<glm::vec2> offsets;
};

#endif
