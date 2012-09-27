#version 330
#include "uniforms.glsl"
#include "screenspace.glsl"
#
uniform vec4 color;

in vec2 uv;
out vec4 ocolor;

void main(){
	ocolor = texture2D(texture0, uv) * color;
}
