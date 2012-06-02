#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

class TextureBase {
public:
	virtual const glm::ivec2& texture_size();

	virtual void texture_bind() const = 0;
	virtual void texture_unbind() const = 0;

protected:
	TextureBase();
	virtual ~TextureBase();

	glm::ivec2 size;
};

class Texture2D: public TextureBase {
public:
	static const unsigned int default_mipmap_level = 5;

	virtual ~Texture2D();

	/**
	 * Preload a texture into memory. Useful during loading sequence.
	 */
	static void preload(const std::string& path);

	/**
	 * Load texture by name (cached if possible)
	 * Paths are relative to texture folder. Textures are cached unless mipmap levels are changed.
	 */
	static Texture2D* from_filename(const std::string &path, const unsigned int num_mipmap_levels = default_mipmap_level);

	/**
	 * Load a default texture.
	 * Automatically called when a file is missing.
	 */
	static Texture2D* default_texture();

	virtual void texture_bind() const;
	virtual void texture_unbind() const;

private:
	Texture2D(const std::string &path, unsigned int num_mipmap_levels);

	GLuint _texture;
	size_t _mipmap_count;
};

class TextureCubemap: public TextureBase {
public:
	virtual ~TextureCubemap();

	/**
	 * The cubemap textures will be loaded in the following order:
	 *	 GL_TEXTURE_CUBE_MAP_POSITIVE_X
	 *	 GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	 *	 GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	 *	 GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	 *	 GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	 *	 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	 */
	static TextureCubemap* from_filename(
		const std::string& px, const std::string& nx,
		const std::string& py, const std::string& ny,
		const std::string& pz, const std::string& nz);
	static TextureCubemap* from_filename(const std::vector<std::string>& paths);

	virtual void texture_bind() const;
	virtual void texture_unbind() const;

private:
	TextureCubemap(std::vector<std::string> paths);

	GLuint _texture;
};

class TextureArray: public TextureBase {
public:
	virtual ~TextureArray();

	static TextureArray* from_filename(const char* filename, ...) __attribute__((sentinel));
	static TextureArray* from_filename(const std::vector<std::string>& paths);

	size_t num_textures() const;
	virtual void texture_bind() const;
	virtual void texture_unbind() const;

private:
	TextureArray(std::vector<std::string> path);

	size_t _num;
	GLuint _texture;
};

// 		/*
// 		 * Requires the texture to be bound!
// 		 *
// 		 * Sets parameters to CLAMP_EDGE and GL_NEAREST (skybox etc)
// 		 */
// 		void set_clamp_params();

// 		unsigned int mipmap_count()  const;

// 		unsigned int num_textures() const;

// 		GLuint texture_type() const;

// 	private:
// 		// Copy not allowed (no body implemented, intentional!)
// 		// Not allowed since deallocating a texture object frees the texture
// 		Image(const Image &other);

// 		void load_texture();
// 		void free_texture();

// 		std::string * _filenames;
// 		GLuint _texture;
// 		int _width;
// 		int _height;
// 		unsigned int _num_textures;
// 		unsigned int _mipmap_count;
// 		GLuint _texture_type;

// 		static GLuint cube_map_index_[6];
// };

struct texture_pack_t {
	Texture2D * texture;
	Texture2D * normal_map;
	Texture2D * specular_map;
	texture_pack_t() : texture(NULL), normal_map(NULL), specular_map(NULL) {};
	~texture_pack_t() {
		if(texture!=NULL)
			delete texture;
		if(normal_map!=NULL)
			delete normal_map;
		if(specular_map!=NULL)
			delete specular_map;
	};

	void bind() {
		if(texture != NULL) {
			glActiveTexture(GL_TEXTURE0);
			texture->texture_bind();
		}
		if(normal_map != NULL) {
			glActiveTexture(GL_TEXTURE1);
			normal_map->texture_bind();
		}
		if(specular_map != NULL) {
			glActiveTexture(GL_TEXTURE2);
			specular_map->texture_bind();
		}
		glActiveTexture(GL_TEXTURE0);
	};

	void unbind() {
		if(texture != NULL) {
			glActiveTexture(GL_TEXTURE0);
			texture->texture_unbind();
		}
		if(normal_map != NULL) {
			glActiveTexture(GL_TEXTURE1);
			normal_map->texture_unbind();
		}
		if(specular_map != NULL) {
			glActiveTexture(GL_TEXTURE2);
			specular_map->texture_unbind();
		}
		glActiveTexture(GL_TEXTURE0);
	};
};

#endif /* TEXTURE_H */
