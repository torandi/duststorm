#version 150

#include "uniforms.glsl"

in vec3 in_position;
in vec2 in_texcoord;
in vec4 in_normal;
in vec4 in_tangent;
in vec4 in_bitangent;

out vec3 position;

void main() {
	vec4 w_pos = modelMatrix * vec4(in_position, 1.0f);

	position = w_pos.xyz;
	gl_Position = projectionViewMatrix * w_pos;
}
