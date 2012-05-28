#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rendertarget.hpp"
#include "engine.hpp"
#include "utils.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

RenderTarget* RenderTarget::stack = nullptr;

RenderTarget::RenderTarget(const glm::ivec2& size, bool alpha, bool depthbuffer, GLenum filter)
	: size(size)
	, id(0)
	, current(0){

	checkForGLErrors("RenderTarget()");

	/* generate projection matrix for this target */
	projection = glm::ortho(0.0f, (float)size.x, 0.0f, (float)size.y, -1.0f, 1.0f);
	projection = glm::scale(projection, glm::vec3(1.0f, -1.0f, 1.0f));
	projection = glm::translate(projection, glm::vec3(0.0f, -(float)size.y, 0.0f));

	glGenFramebuffers(1, &id);
	glGenTextures(2, color);
	glGenTextures(1, &depth);

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	Engine::setup_opengl();

	/* bind color buffers */
	for ( int i = 0; i < 2; i++ ){
		glBindTexture(GL_TEXTURE_2D, color[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA8 : GL_RGB8, size.x, size.y, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color[current], 0);
	checkForGLErrors("glFramebufferTexture2D::color");

	/* bind depth buffer */
	if ( depth ){
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		checkForGLErrors("glFramebufferTexture2D::depth");
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE){
		fprintf(stderr, "Framebuffer incomplete: %s\n", gluErrorString(status));
		abort();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkForGLErrors("RenderTarget() fin");
}

RenderTarget::~RenderTarget(){
	glDeleteFramebuffers(1, &id);
	glDeleteTextures(2, color);
	glDeleteTextures(1, &depth);
}

void RenderTarget::bind(){
	if ( stack ){
		fprintf(stderr, "Nesting problem with RenderTarget, another target already bound.\n");
		abort();
	}

	glViewport(0, 0, size.x, size.y);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color[current], 0);

	stack = this;
}

void RenderTarget::unbind(){
	if ( !stack ){
		fprintf(stderr, "Nesting problem with RenderTarget, no target is bound\n");
		abort();
	}

	current = 1 - current;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	stack = nullptr;
}

void RenderTarget::with(const std::function<void()>& func){
	bind();
	func();
	unbind();
}

GLuint RenderTarget::texture() const {
	return color[1-current];
}

GLuint RenderTarget::depthbuffer() const {
	return depth;
}

const glm::mat4& RenderTarget::ortho() const {
	return projection;
}

void RenderTarget::clear(const Color& color){
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void RenderTarget::draw(Shader* shader){
	draw(shader, glm::ivec2(0,0), size);
}

void RenderTarget::draw(Shader* shader, const glm::ivec2& pos){
	draw(shader, pos, size);
}

void RenderTarget::draw(Shader* shader, const glm::ivec2& pos, const glm::ivec2& size){
	static const float vertices[][5] = { /* x,y,z,u,v */
		{0, 0, 0, 0, 1},
		{0, 1, 0, 0, 0},
		{1, 1, 0, 1, 0},
		{1, 0, 0, 1, 1},
	};
	static const unsigned int indices[4] = {0,1,2,3};

	glm::mat4 model(1.f);

	model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

	Shader::upload_model_matrix(model);

	shader->bind();
	glBindTexture(GL_TEXTURE_2D, texture());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5,  &vertices[0][0]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5,  &vertices[0][3]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, indices);
}
