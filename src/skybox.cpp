#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <GL/glew.h>

#include "skybox.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "utils.hpp"


Skybox::Skybox(std::string skybox_path) {

	//Load skybox texture:
	skybox_path+="/";

	std::vector<std::string> files;
	for(int i=0; i < 6; ++i) {
		files.push_back(skybox_path+texture_names[i]);
	}

	texture = TextureCubemap::from_filename(files);

	//Generate skybox buffers:
	static bool initialized = false;
	if ( !initialized ){
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		initialized = true;
	}
}

Skybox::~Skybox() {
	delete texture;
}

void Skybox::render(const Camera &camera) const{
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	Shader::upload_projection_view_matrices(
			camera.projection_matrix(),
			glm::lookAt(glm::vec3(0.0), camera.look_at()-camera.position(), camera.up())
	);

	/* Disable most attribs from Shader::vertex_x */
	for ( int i = 2; i < Shader::NUM_ATTR; ++i ) {
		glDisableVertexAttribArray(i);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*36) );

	glActiveTexture(Shader::TEXTURE_CUBEMAP_0);

	texture->texture_bind();

	glDrawArrays(GL_TRIANGLES, 0, 36);

	checkForGLErrors("Skybox::render(): render");

	texture->texture_unbind();

	glPopAttrib();
	glPopClientAttrib();

	checkForGLErrors("Skybox::render(): post");
}

GLuint Skybox::vbo = 0;

//Data:
const float Skybox::vertices[] = {

	//Front
	0.5f, 0.5f, 0.5f,
	0.5f, -0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,

	0.5f, -0.5f, 0.5f,
	-0.5f, -0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,

	//Back
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,

	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,

	//Left
	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	//Right
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,

	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,

	//Top
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, 0.5f,

	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, 0.5f,

	//Bottom
	0.5f, -0.5f, 0.5f,
	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	//UV Coords

	//Front
	0.5f, 0.5f, 0.5f,
	0.5f, -0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,

	0.5f, -0.5f, 0.5f,
	-0.5f, -0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,

	//Back
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,

	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,

	//Left
	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	//Right
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,

	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,

	//Top
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, 0.5f,

	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, 0.5f,

	//Bottom
	0.5f, -0.5f, 0.5f,
	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

	0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,

};

const char * Skybox::texture_names[] {
	"left.png",
	"right.png",
	"top.png",
	"bottom.png",
	"front.png",
	"back.png"
};

