#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "color.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>

class RenderTarget {
public:
	static RenderTarget* stack;

	RenderTarget(const glm::ivec2& size, bool alpha);
	~RenderTarget();

	void bind();
	void unbind();

	GLuint texture() const;

	void clear(const Color& color) const;

	const glm::ivec2 size;
	GLuint id;
	GLuint current;
	GLuint color[2];
};

#endif /* RENDER_TARGET_H */
