#version 130

#include "uniforms.glsl"

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_normal;
in vec4 in_tangent;
in vec4 in_bitangent;

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
