#version 150
#include "uniforms.glsl"

in vec4 color;
in vec2 tex_coord;
in int texture_index;

out vec4 ocolor;

void main() {
	vec4 tex_color = texture2DArray(texture_array1, vec3(tex_coord, texture_index));
	ocolor = tex_color*color;
}
