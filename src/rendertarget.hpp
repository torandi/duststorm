#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "color.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <functional>

class RenderTarget: public TextureBase {
public:
	static RenderTarget* stack;

	explicit RenderTarget(const glm::ivec2& size, GLenum format, bool depth, GLenum filter = GL_NEAREST);
	~RenderTarget();

	void bind();
	void unbind();

	/**
	 * Get orthographic projection for rendering on this target.
	 */
	const glm::mat4& ortho() const;

	/**
	 * Call func while target is bound.
	 * short for: bind(); func(); unbind();
	 */
	void with(const std::function<void()>& func);

	/**
	 * Get texture id of current frontbuffer.
	 */
	GLuint texture() const;

	virtual void texture_bind() const;
	virtual void texture_unbind() const;

	/**
	 * Get texture id of depthbuffer.
	 */
	GLuint depthbuffer() const;

	static void clear(const Color& color);

	/**
	 * Render the RenderTarget on current framebuffer.
	 */
	void draw(Shader* shader);
	void draw(Shader* shader, const glm::vec2& pos);
	void draw(Shader* shader, const glm::ivec2& pos);
	void draw(Shader* shader, const glm::ivec2& pos, const glm::ivec2& size);
	void draw(Shader* shader, const glm::vec2& pos, const glm::vec2& size);

	glm::mat4 projection;
	GLuint id;
	GLuint current;
	GLuint color[2];
	GLuint depth;
};

#endif /* RENDER_TARGET_H */
