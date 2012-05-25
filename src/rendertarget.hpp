#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "color.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <functional>

class RenderTarget {
public:
	static RenderTarget* stack;

	RenderTarget(const glm::ivec2& size, bool alpha, bool depth);
	~RenderTarget();

	void bind();
	void unbind();

	/**
	 * Call func while target is bound.
	 * short for: bind(); func(); unbind();
	 */
	void with(const std::function<void()>& func);

	/**
	 * Get texture id of current frontbuffer.
	 */
	GLuint texture() const;

	/**
	 * Get texture id of depthbuffer.
	 */
	GLuint depthbuffer() const;

	void clear(const Color& color) const;

	/**
	 * Render the RenderTarget on current framebuffer.
	 */
	void draw();
	void draw(const glm::ivec2& pos);
	void draw(const glm::ivec2& pos, const glm::ivec2& size);

	const glm::ivec2 size;
	GLuint id;
	GLuint current;
	GLuint color[2];
	GLuint depth;
};

#endif /* RENDER_TARGET_H */
