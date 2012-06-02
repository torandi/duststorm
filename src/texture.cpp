#include "texture.hpp"
#include "utils.hpp"
#include "globals.hpp"

#include <vector>
#include <map>
#include <string>
#include <cstdarg>
#include <cassert>

GLuint Texture::cube_map_index_[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

static std::map<std::string, Texture*> texture_cache;

Texture* Texture::mipmap(const std::string &path, const unsigned int num_mipmap_levels) {
	/* search cache */
	auto it = texture_cache.find(path);
	if ( it != texture_cache.end() ){
		return it->second;
	}

	/* create new instance */
	const std::string real_path = std::string(PATH_BASE "textures/") + path;
	Texture* texture = new Texture(real_path, num_mipmap_levels);
	texture_cache[path] = texture;

	return texture;
}

Texture * Texture::cubemap(
		std::string px, std::string nx,
		std::string py, std::string ny,
		std::string pz, std::string nz) {
	std::vector<std::string> v;
	v.push_back(px);
	v.push_back(nx);
	v.push_back(py);
	v.push_back(ny);
	v.push_back(pz);
	v.push_back(nz);
	return new Texture(v, true);
}

Texture * Texture::array(std::vector<std::string> &paths) {
	return new Texture(paths, false);
}

Texture * Texture::array(int num_textures, ...) {
	std::vector<std::string> textures;
	va_list list;
	va_start(list, num_textures);
	for(int i=0; i < num_textures; ++i) {
		textures.push_back(va_arg(list, const char *));
	}
	va_end(list);
	return new Texture(textures, false);
}

Texture::Texture(const std::string &path, const unsigned int num_mipmap_levels) :
	_texture(-1),
	_width(0),
	_height(0),
	_num_textures(1),
	_mipmap_count(num_mipmap_levels)
	{
	_filenames = new std::string[_num_textures];
	_filenames[0] = path;
	_texture_type = GL_TEXTURE_2D;
	load_texture();
}

Texture::Texture(const std::vector<std::string> &paths, bool cube_map) :
	_texture(-1),
	_width(0),
	_height(0),
	_num_textures(paths.size()),
	_mipmap_count(1)
	{
	if(cube_map) {
		assert(_num_textures == 6);
		_texture_type = GL_TEXTURE_CUBE_MAP;
	} else
		_texture_type = GL_TEXTURE_2D_ARRAY;
	_filenames = new std::string[_num_textures];
	int i=0;
	for(std::vector<std::string>::const_iterator it=paths.begin(); it!=paths.end(); ++it) {
		_filenames[i++] = (*it);
	}
	load_texture();
}

Texture::~Texture(){
	delete[] _filenames;
	free_texture();
}

int Texture::width() const {
	return _width;
}

int Texture::height() const {
	return _height;
}

unsigned int Texture::mipmap_count() const {
	return _mipmap_count;
}

unsigned int Texture::num_textures() const {
	return _num_textures;
}

GLuint Texture::texture_type() const {
	return _texture_type;
}

void Texture::bind() const {
	assert(_texture != (unsigned int)-1);
	glBindTexture(_texture_type, _texture);
	checkForGLErrors(_filenames[0].c_str());
}

void Texture::unbind() const {
	glBindTexture(_texture_type, 0);
}

GLuint Texture::texture() const {
	return _texture;
}

void Texture::load_texture() {
	assert(_texture == (unsigned int)-1);
	//Load textures:
	SDL_Surface ** images = new SDL_Surface*[_num_textures];
	for(unsigned int i=0; i < _num_textures; ++i) {
		images[i] = load_image(_filenames[i]);
	}
	_width = images[0]->w;
	_height = images[0]->h;

	//Generate texture:
	glGenTextures(1, &_texture);
	bind();
	glTexParameteri(_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	checkForGLErrors("[Texture] load_texture(): gen buffer");

	switch(_texture_type) {
		case GL_TEXTURE_2D:
			{
				//One texture only:
				GLint err = gluBuild2DMipmapLevels(GL_TEXTURE_2D, GL_RGBA, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, 0, 0, _mipmap_count, images[0]->pixels );
				if(err != 0) {
					fprintf(verbose, "[Texture] gluBuild2DMipmapLevels for %s return %s\n", _filenames[0].c_str(), gluErrorString(err));
					abort();
				}
				break;
			}
		case GL_TEXTURE_2D_ARRAY:
			//Generate the array:
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, _width, _height,
				_num_textures, 0, GL_RGBA,  GL_UNSIGNED_BYTE, NULL);
			checkForGLErrors("[Texture] load_texture(): gen 2d array buffer");

			//Fill the array with data:
			for(unsigned int i=0; i < _num_textures; ++i) {
				//											, lvl, x, y, z, width, height, depth

				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, images[i]->w, images[i]->h, 1, GL_RGBA, GL_UNSIGNED_BYTE, images[i]->pixels);
				checkForGLErrors("[Texture] load_texture(): glTexSubImage3D");
			}
			break;
		case GL_TEXTURE_CUBE_MAP:
			set_clamp_params();
			for(int i=0; i < 6; ++i) {
				assert(_width == _height);
				glTexImage2D(cube_map_index_[i], 0, GL_RGBA , _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, images[i]->pixels );
				checkForGLErrors("[Texture] load_texture(): Fill cube map");
			}
			break;
		default:
			fprintf(verbose, "[Texture] Error! Invalid texture type encountered when loading textures, exiting (Texture::load_texture()");
			abort();
	}

	/*if(_mipmap_count > 0) {
		glTexParameteri(_texture_type, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(_texture_type, GL_TEXTURE_MAX_LEVEL, _mipmap_count - 1);
	}*/

	unbind();

	//Free images:
	for(unsigned int i=0; i<_num_textures; ++i) {
		SDL_FreeSurface(images[i]);
	}
	delete[] images;
}

//Requires the texture to be bound!
void Texture::set_clamp_params() {
	glTexParameteri(_texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(_texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(_texture_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(_texture_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(_texture_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

SDL_Surface * Texture::load_image(const std::string &path) {
	/* Load image using SDL Image */
	SDL_Surface* surface = IMG_Load(path.c_str());
	if ( !surface ){
		fprintf(stderr, "Failed to load texture at %s\n", path.c_str());
		abort();
	}

	/* To properly support all formats the surface must be copied to a new
	 * surface with a prespecified pixel format suitable for opengl.
	 *
	 * This snippet is a slightly modified version of code posted by
	 * Sam Lantinga to the SDL mailinglist at Sep 11 2002.
	 */
	SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
			SDL_HWSURFACE,
			surface->w, surface->h,
			32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000
#else
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
#endif
	);

	if ( !rgba_surface ) {
		fprintf(stderr, "Failed to create RGBA surface\n");
		abort();
	}

	/* Save the alpha blending attributes */
	Uint32 saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
	Uint8 saved_alpha = surface->format->alpha;
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, 0, 0);
	}

	SDL_BlitSurface(surface, 0, rgba_surface, 0);

	/* Restore the alpha blending attributes */
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, saved_flags, saved_alpha);
	}


	SDL_FreeSurface(surface);

	return rgba_surface;
}

void Texture::free_texture(){
	if(_texture != (unsigned int)-1) {
		glDeleteTextures(1, &_texture);
		_texture = -1;
	}
}
