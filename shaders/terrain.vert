#version 330
#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec4 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_bitangent;

out vec3 position;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;
out vec2 texcoord;

void main() {
	vec4 w_pos = modelMatrix * in_position;
	position = w_pos.xyz;
	gl_Position = projectionViewMatrix *  w_pos;
	texcoord = in_texcoord;
	normal = (normalMatrix * in_normal).xyz;
	tangent = (normalMatrix * in_tangent).xyz;
	bitangent = (normalMatrix * in_bitangent).xyz;
}

