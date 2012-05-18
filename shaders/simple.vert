#version 130

#include "uniforms.glsl"

in vec4 in_position;

out vec3 position;

void main() {
	vec4 w_pos = modelMatrix * in_position;
	position = w_pos.xyz;
	gl_Position = projectionViewMatrix *  w_pos;
}
