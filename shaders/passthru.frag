#version 330
#include "uniforms.glsl"
#include "screenspace.glsl"

out vec4 ocolor;

void main(){
	vec2 uv = screenspace_uv(texture0);
	ocolor = texture2D(texture0, uv);
}
