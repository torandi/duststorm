#ifndef MATERIAL_H
#define MATERIAL_H

#include "shader.hpp"
#include "texture.hpp"

class Material: public Shader::material_t {
public:
	Material();
	~Material();

	/**
	 * Upload material attributes and bind texture units */
	void bind() const;

	TextureBase* texture;
	TextureBase* normal_map;
	TextureBase* specular_map;
	TextureBase* alpha_map;
};

#endif /* MATERIAL_H */
