#version 150
#extension GL_EXT_gpu_shader4 : enable
#include "uniforms.glsl"

in vec4 color;
in vec2 tex_coord;
flat int texture_index;

out vec4 ocolor;

void main() {
	vec4 tex_color = texture2DArray(texture_array1, vec3(tex_coord, texture_index));
	ocolor = tex_color*color;
}
