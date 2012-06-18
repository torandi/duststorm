#version 330
#include "uniforms.glsl"
#include "screenspace.glsl"

in vec2 uv;
out vec4 ocolor;

void main(){
	ocolor = texture2D(texture0, uv);
}
