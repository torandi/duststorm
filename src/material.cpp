#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "material.hpp"
#include <GL/glew.h>

Material::Material()
	: texture(nullptr)
	, normal_map(nullptr)
	, two_sided(false){

	shininess = 1;
	diffuse   = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	specular  = glm::vec4(0.0f);
	ambient   = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	emission  = glm::vec4(0.0f);
};

Material::~Material(){

}

void Material::activate() {
	glPushAttrib(GL_ENABLE_BIT);
	if(two_sided)
		glDisable(GL_CULL_FACE);

	texture->texture_bind(Shader::TEXTURE_COLORMAP);
	normal_map->texture_bind(Shader::TEXTURE_NORMALMAP);

	Shader::upload_material(*this);
}

void Material::deactivate() {
	glPopAttrib();
}
