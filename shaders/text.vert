#version 330
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

uniform vec2 font_offset;

layout (location=0) in vec4 in_pos;
layout (location=1) in vec2 in_uv;
layout (location = 2) in vec4 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_bitangent;

out vec2 uv;

void main(){
	uv = in_uv + font_offset;
	vec4 w_pos = modelMatrix * in_pos;
	gl_Position = projectionViewMatrix *  w_pos;
}
