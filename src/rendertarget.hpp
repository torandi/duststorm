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

	/**
	 * RenderTarget is a framebuffer object which you can render to.
	 *
	 * Typical usage:
	 *
	 *   RenderTarget foo(ivec2(200,200), GL_RGB8, true);
	 *   foo.bind();
	 *   // render code here..
	 *   foo.unbind();
	 *   foo.draw()
	 *
	 * Color attachment is double-buffered so it is safe to bind itself as texture
	 * unit when rendering. Depth-buffer is not double buffered and should not be
	 * used the same way.
	 *
	 * @param size Size of the final textures.
	 * @param format Format of the color attachment.
	 * @param depth If true, it enables depth buffer/write for the framebuffer.
	 * @param filter Texture filtering of color attachment.
	 */
	explicit RenderTarget(const glm::ivec2& size, GLenum format, bool depth, GLenum filter = GL_NEAREST) throw();
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

	virtual void texture_bind(Shader::TextureUnit unit) const;
	virtual void texture_unbind() const;

	void depth_bind(Shader::TextureUnit unit) const;
	void depth_unbind() const;

	/**
	 * Get texture id of depthbuffer.
	 */
	GLuint depthbuffer() const;

	/**
	 * Short for: glClearColor(..); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	 */
	static void clear(const Color& color);

	/**
	 * Render the RenderTarget on current framebuffer. Caller should ensure an
	 * orthographic projection is bound before calling draw.
	 *
	 * Always passes colormap as texture unit 0.
	 */
	void draw(Shader* shader);
	void draw(Shader* shader, const glm::vec2& pos);
	void draw(Shader* shader, const glm::ivec2& pos);
	void draw(Shader* shader, const glm::ivec2& pos, const glm::ivec2& size);
	void draw(Shader* shader, const glm::vec2& pos, const glm::vec2& size);

private:
	static GLuint vbo[2];
	static void init_vbo();

	glm::mat4 projection;
	GLuint id;
	GLuint current;
	GLuint color[2];
	GLuint depth;
};

#endif /* RENDER_TARGET_H */
