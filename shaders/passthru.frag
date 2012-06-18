#version 330
#include "uniforms.glsl"
#include "screenspace.glsl"

out vec4 ocolor;

void main(){
	vec2 uv = screenspace_uv();
	ocolor = texture2D(texture0, uv);
}
