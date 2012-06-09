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

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	texture = TextureCubemap::from_filename(files);

	//Generate skybox buffers:

	glGenBuffers(1, &buffer_);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Skybox::~Skybox() {
	delete texture;		
}

void Skybox::render(const Camera &camera) const{
	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	Shader::upload_projection_view_matrices(
			camera.projection_matrix(), 
			glm::lookAt(glm::vec3(0.0), camera.look_at()-camera.position(), camera.up())
	);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*36) );

	glActiveTexture(GL_TEXTURE2);

	texture->texture_bind();

	glDrawArrays(GL_TRIANGLES, 0, 36);

	checkForGLErrors("Skybox::render(): render");

	texture->texture_unbind();

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glPopAttrib();

	checkForGLErrors("Skybox::render(): post");
}

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

