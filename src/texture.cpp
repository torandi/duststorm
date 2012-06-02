#include "texture.hpp"
#include "utils.hpp"
#include "globals.hpp"

#include <vector>
#include <map>
#include <string>
#include <cstdarg>
#include <cassert>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

static GLuint cube_map_index[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

static SDL_Surface* load_image(const std::string &path, glm::ivec2* size) {
	const std::string real_path = std::string(PATH_BASE "textures/") + path;

	/* Load image using SDL Image */
	fprintf(verbose, "Loading image `%s'\n", real_path.c_str());
	SDL_Surface* surface = IMG_Load(real_path.c_str());
	if ( !surface ){
		fprintf(stderr, "Failed to load texture at %s\n", real_path.c_str());
		if ( path != "default.jpg" ){
			return load_image("default.jpg", size);
		}
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

	/* save image resolution */
	size->x = surface->w;
	size->y = surface->h;

	SDL_FreeSurface(surface);

	return rgba_surface;
}

TextureBase::TextureBase()
	: size(0,0) {

}

TextureBase::~TextureBase(){

}

const glm::ivec2& TextureBase::texture_size(){
	return size;
}

static std::map<std::string, Texture2D*> texture_cache;

void Texture2D::preload(const std::string& path){
	from_filename(path);
};

Texture2D* Texture2D::from_filename(const std::string &path, const unsigned int num_mipmap_levels) {
	/* if custom mipmap levels has been set texture cannot be cached */
	if ( num_mipmap_levels != default_mipmap_level ){
		return new Texture2D(path, num_mipmap_levels);
	}

	/* search cache */
	auto it = texture_cache.find(path);
	if ( it != texture_cache.end() ){
		return it->second;
	}

	/* create new instance */
	Texture2D* texture = new Texture2D(path, num_mipmap_levels);
	texture_cache[path] = texture;

	return texture;
}

Texture2D* Texture2D::default_texture(){
	return from_filename("default.jpg");
}

Texture2D::Texture2D(const std::string& filename, unsigned int num_mipmap_levels)
	: TextureBase()
	, _texture(0)
	, _mipmap_count(num_mipmap_levels) {

	SDL_Surface* image = load_image(filename, &size);

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(image);
}

Texture2D::~Texture2D(){
	glDeleteTextures(1, &_texture);
}

void Texture2D::texture_bind() const {
	glBindTexture(GL_TEXTURE_2D, _texture);
}

void Texture2D::texture_unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

TextureCubemap* TextureCubemap::from_filename(
	const std::string& px, const std::string& nx,
	const std::string& py, const std::string& ny,
	const std::string& pz, const std::string& nz) {

	std::vector<std::string> v;
	v.push_back(px);
	v.push_back(nx);
	v.push_back(py);
	v.push_back(ny);
	v.push_back(pz);
	v.push_back(nz);

	return new TextureCubemap(v);
}

TextureCubemap* TextureCubemap::from_filename(const std::vector<std::string>& paths){
	return new TextureCubemap(paths);
}

TextureCubemap::TextureCubemap(std::vector<std::string> path)
	: TextureBase()
	, _texture(0) {

	/* ensure we got correct number of filenames */
	if ( path.size() != 6 ){
		fprintf(stderr, "TextureCubemap requires 6 filenames, got %zd\n", path.size());
		while ( path.size() < 6 ) path.push_back("default.jpg");
	}

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for ( size_t i = 0; i < 6; i++ ){
		SDL_Surface* surface = load_image(path[i], &size);
		glTexImage2D(cube_map_index[i], 0, GL_RGBA , size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		SDL_FreeSurface(surface);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

TextureCubemap::~TextureCubemap(){
	glDeleteTextures(1, &_texture);
}

void TextureCubemap::texture_bind() const {
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
}

void TextureCubemap::texture_unbind() const {
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

TextureArray* TextureArray::from_filename(const char* filename, ...){
	std::vector<std::string> paths;
	va_list ap;
	va_start(ap, filename);
	while ( filename ){
		paths.push_back(filename);
		filename = va_arg(ap, const char*);
	}
	va_end(ap);

	return new TextureArray(paths);
}

TextureArray* TextureArray::from_filename(const std::vector<std::string>& paths) {
	return new TextureArray(paths);
}

TextureArray::TextureArray(std::vector<std::string> path)
	: TextureBase()
	, _num(path.size())
	, _texture(0) {

	if ( path.size() == 0 ){
		fprintf(stderr, "TextureArray must have at least one image, got 0.\n");
		path.push_back("default.jpg");
		_num++;
	}

	/* hack to get size */
	{
		SDL_Surface* surface = load_image(path[0], &size);
		SDL_FreeSurface(surface);
	}

	fprintf(verbose, "Creating TextureArray with %zd images at %dx%d\n", path.size(), size.x, size.y);

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, size.x, size.y, num_textures(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	int n = 0;
	for ( const std::string& filename: path) {
		glm::ivec2 cur_size;
		SDL_Surface* surface = load_image(filename, &cur_size);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, n++, cur_size.x, cur_size.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		SDL_FreeSurface(surface);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

TextureArray::~TextureArray(){
	glDeleteTextures(1, &_texture);
}

size_t TextureArray::num_textures() const {
	return _num;
}

void TextureArray::texture_bind() const {
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
}

void TextureArray::texture_unbind() const {
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
