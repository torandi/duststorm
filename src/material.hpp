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
	void activate();

	/**
	 * Restore state if needed.
	 */
	void deactivate();

	TextureBase* texture;
	TextureBase* normal_map;
	bool two_sided;
};

#endif /* MATERIAL_H */
