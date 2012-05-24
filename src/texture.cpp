#include "renderer.h"
#include "texture.h"

#include <glimg/glimg.h>
#include <vector>
#include <string>
#include <cassert>

GLuint Texture::cube_map_index_[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 
};

Texture::Texture(const std::string &path) :
	_texture(-1),
	_width(0),
	_height(0),
	_num_textures(1),
	_mipmap_count(1)
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
	_num_textures(paths.size())
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

void Texture::bind() const {
	assert(_texture != (unsigned int)-1);
	glBindTexture(_texture_type, _texture);
	char tmp[256];
	sprintf(tmp, "Texture(%s,...)::bind()", _filenames[0].c_str());
	Renderer::checkForGLErrors(tmp);
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
	glimg::ImageSet ** images = new glimg::ImageSet*[_num_textures];
	for(unsigned int i=0; i < _num_textures; ++i) {
		images[i] = load_image(_filenames[i]);
	}
	_width = images[0]->GetDimensions().width;
	_height = images[0]->GetDimensions().height;

	//Generate texture:
	glGenTextures(1, &_texture);
	bind();	
	glTexParameteri(_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	Renderer::checkForGLErrors("load_texture(): gen buffer");

	glimg::OpenGLPixelTransferParams fmt;

	switch(_texture_type) {
		case GL_TEXTURE_2D:
			//One texture only:
			glPixelStorei(GL_UNPACK_ALIGNMENT, images[0]->GetFormat().LineAlign());

			_mipmap_count = images[0]->GetMipmapCount();

			for(unsigned int mipmap_lvl=0; mipmap_lvl<_mipmap_count; ++mipmap_lvl) {
				const glimg::SingleImage &img = images[0]->GetImage(mipmap_lvl, 0, 0);
				const glimg::Dimensions &dim = img.GetDimensions();
				fmt = glimg::GetUploadFormatType(img.GetFormat(), 0); 
				glTexImage2D(GL_TEXTURE_2D, mipmap_lvl, GL_RGBA, dim.width, dim.height, 
					0, fmt.format, fmt.type, img.GetImageData() );
			}
			Renderer::checkForGLErrors("load_texture(): write GL_TEXTURE_2D data");
			break;
		case GL_TEXTURE_2D_ARRAY:
			 fmt = glimg::GetUploadFormatType(images[0]->GetFormat(), 0); 
			//Generate the array:
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, _width, _height,
				_num_textures, 0,   fmt.format,  fmt.type, NULL);
			Renderer::checkForGLErrors("load_texture(): gen 2d array buffer");

			_mipmap_count = images[0]->GetMipmapCount();

			//Fill the array with data:
			for(unsigned int i=0; i < _num_textures; ++i) {
				//											, lvl, x, y, z, width, height, depth
				glPixelStorei(GL_UNPACK_ALIGNMENT, images[i]->GetFormat().LineAlign());
				if(_mipmap_count > images[i]->GetMipmapCount())
					_mipmap_count = images[i]->GetMipmapCount(); //Find smallest value

				for(unsigned int mipmap_lvl=0; mipmap_lvl<_mipmap_count; ++mipmap_lvl) {
					const glimg::SingleImage &img = images[i]->GetImage(mipmap_lvl, 0, 0);
					const glimg::Dimensions &dim = img.GetDimensions();
					fmt = glimg::GetUploadFormatType(img.GetFormat(), 0); 
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipmap_lvl, 0, 0, i, dim.width, dim.height, 1, fmt.format,  fmt.type,
						img.GetImageData());
				}
			}
			break;
		case GL_TEXTURE_CUBE_MAP:
			set_clamp_params();
			for(int i=0; i < 6; ++i) {
				fmt = glimg::GetUploadFormatType(images[i]->GetFormat(), 0); 
				glPixelStorei(GL_UNPACK_ALIGNMENT, images[i]->GetFormat().LineAlign());
				assert(_width == _height);
				glTexImage2D(cube_map_index_[i], 0, GL_RGBA , _width, _height, 0, fmt.format, fmt.type, images[i]->GetImageArray(0) );
				Renderer::checkForGLErrors("load_texture(): Fill cube map");
			}
			break;
		default:
			fprintf(stderr, "Error! Invalid texture type encountered when loading textures, exiting (Texture::load_texture()");
			assert(false);
	}

	/*if(_mipmap_count > 0) {
		glTexParameteri(_texture_type, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(_texture_type, GL_TEXTURE_MAX_LEVEL, _mipmap_count - 1);
	}*/

	unbind();

	//Free images:
	for(unsigned int i=0; i<_num_textures; ++i) {
		delete images[i];
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

glimg::ImageSet * Texture::load_image(const std::string &path) {
	//Check file format:
	if(path.substr(path.length()-4) == ".dds") {
		printf("Using dds loader for %s\n",path.c_str());
		return glimg::loaders::dds::LoadFromFile(path.c_str());
	} else {
		return glimg::loaders::stb::LoadFromFile(path.c_str());
	}
}

void Texture::free_texture(){
	if(_texture != (unsigned int)-1) {
		glDeleteTextures(1, &_texture);
		_texture = -1;
	}
}
