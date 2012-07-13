#include "texture.hpp"
#include "utils.hpp"
#include "globals.hpp"
#include "data.hpp"

#include <vector>
#include <map>
#include <string>
#include <cstdarg>
#include <cassert>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <glm/glm.hpp>

static GLuint cube_map_index[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

SDL_Surface* TextureBase::load_image(const std::string &path, glm::ivec2* size) {
	const std::string real_path = std::string(PATH_BASE "game/data/") + path;

	/* Load image using SDL Image */
	fprintf(verbose, "Loading image `%s'\n", real_path.c_str());
	Data * file = Data::open(real_path);

	if ( !file ){
		fprintf(stderr, "Failed to load texture at %s\n", real_path.c_str());
		if ( path != "default.jpg" ){
			return load_image("default.jpg", size);
		}
		abort();
	}

	SDL_RWops * rw = SDL_RWFromConstMem(file->data(), file->size());
	SDL_Surface* surface = IMG_Load_RW(rw, 1);
	delete file;

	if ( !surface ){
		fprintf(stderr, "Failed to load surface from `%s'\n", real_path.c_str());
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

	/* flip image correct (SDL used flipped images) */
	uint8_t* t = (uint8_t*)malloc(rgba_surface->pitch);
	int middle = (int)(rgba_surface->h * 0.5f);
	for ( int i = 0; i < middle; i++ ){
		uint8_t* a = (uint8_t*)rgba_surface->pixels + rgba_surface->pitch * i;
		uint8_t* b = (uint8_t*)rgba_surface->pixels + rgba_surface->pitch * (rgba_surface->h - i-1);

		memcpy(t, a, rgba_surface->pitch);
		memcpy(a, b, rgba_surface->pitch);
		memcpy(b,	t, rgba_surface->pitch);
	}

	free(t);
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

Texture2D* Texture2D::default_colormap(){
	return from_filename("default.jpg");
}

Texture2D* Texture2D::default_normalmap(){
	return from_filename("default_normalmap.png");
}

Texture2D* Texture2D::default_specularmap(){
	return from_filename("white.png");
}

Texture2D* Texture2D::default_alphamap(){
	return from_filename("white.png");
}

Texture2D::Texture2D(const std::string& filename, unsigned int num_mipmap_levels)
	: TextureBase()
	, _texture(0)
	, _mipmap_count(num_mipmap_levels) {

	SDL_Surface* image = load_image(filename, &size);

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(image);
}

Texture2D::~Texture2D(){
	glDeleteTextures(1, &_texture);
}

void Texture2D::texture_bind(Shader::TextureUnit unit) const {
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D, _texture);
}

void Texture2D::texture_unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

const GLint Texture2D::gl_texture() const {
	return _texture;
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

void TextureCubemap::texture_bind(Shader::TextureUnit unit) const {
	glActiveTexture(unit);
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
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
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

void TextureArray::texture_bind(Shader::TextureUnit unit) const {
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
}

void TextureArray::texture_unbind() const {
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

/*
 * 3D texture
 */

Texture3D* Texture3D::from_filename(const char* filename, ...){
	std::vector<std::string> paths;
	va_list ap;
	va_start(ap, filename);
	while ( filename ){
		paths.push_back(filename);
		filename = va_arg(ap, const char*);
	}
	va_end(ap);

	return new Texture3D(paths);
}

Texture3D* Texture3D::from_filename(const std::vector<std::string>& paths) {
	return new Texture3D(paths);
}

Texture3D::Texture3D(std::vector<std::string> path)
	: TextureBase()
	, _texture(0)
	, _depth(path.size()) {

	if ( path.size() == 0 ){
		fprintf(stderr, "Texture3D must have at least one image, got 0.\n");
		path.push_back("default.jpg");
		_depth++;
	}

	/* hack to get size */
	{
		SDL_Surface* surface = load_image(path[0], &size);
		SDL_FreeSurface(surface);
	}

	fprintf(verbose, "Creating Texture3D with %zd images at %dx%d\n", path.size(), size.x, size.y);

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_3D, _texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, size.x, size.y, depth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	int n = 0;
	for ( const std::string& filename: path) {
		glm::ivec2 cur_size;
		SDL_Surface* surface = load_image(filename, &cur_size);
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, n++, cur_size.x, cur_size.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		SDL_FreeSurface(surface);
	}

	glBindTexture(GL_TEXTURE_3D, 0);
}

Texture3D::~Texture3D(){
	glDeleteTextures(1, &_texture);
}

const int Texture3D::depth() const {
	return _depth;
}

void Texture3D::texture_bind(Shader::TextureUnit unit) const {
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_3D, _texture);
}

void Texture3D::texture_unbind() const {
	glBindTexture(GL_TEXTURE_3D, 0);
}

const GLint Texture3D::gl_texture() const {
	return _texture;
}

glm::ivec4 TextureBase::get_pixel_color(int x, int y, SDL_Surface * surface, const glm::ivec2 &size) {
	Uint32 temp, pixel;
	Uint8 red, green, blue, alpha;
	pixel = ((Uint32*)surface->pixels)[(size.y-(y+1))*(size.x)+x];

	SDL_PixelFormat * fmt=surface->format;

	/* Get Red component */
	temp = pixel & fmt->Rmask;  /* Isolate red component */
	temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
	red = (Uint8)temp;

	/* Get Green component */
	temp = pixel & fmt->Gmask;  /* Isolate green component */
	temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
	green = (Uint8)temp;

	/* Get Blue component */
	temp = pixel & fmt->Bmask;  /* Isolate blue component */
	temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
	temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
	blue = (Uint8)temp;

	/* Get Alpha component */
	temp = pixel & fmt->Amask;  /* Isolate alpha component */
	temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
	temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
	alpha = (Uint8)temp;
	return glm::ivec4(red, green, blue, alpha);
}
