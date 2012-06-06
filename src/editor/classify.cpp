#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor/editor.hpp"
#include <cctype>
#include <algorithm>

namespace Editor {

TYPE classify_name(const std::string&name, std::string& data){
	const size_t offset = name.find_first_of(":");
	std::string prefix = name;
	data = "";

	if ( offset != std::string::npos ){
		prefix = name.substr(0, offset);
		data = name.substr(offset+1);
	}

	std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);

	if ( prefix == "path"     ) return TYPE_PATH;
	if ( prefix == "model"    ) return TYPE_MODEL;
	if ( prefix == "light"    ) return TYPE_LIGHT;
	if ( prefix == "int"      ) return TYPE_INT;
	if ( prefix == "float"    ) return TYPE_FLOAT;
	if ( prefix == "vec2"     ) return TYPE_VEC2;
	if ( prefix == "vec3"     ) return TYPE_VEC3;
	if ( prefix == "vec4"     ) return TYPE_VEC4;
	if ( prefix == "string"   ) return TYPE_STRING;

	data = name;
	return TYPE_UNKNOWN;
}

}
