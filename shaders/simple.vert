#version 130

#include "uniforms.glsl"

in vec3 in_position;

out vec3 position;

void main() {
	//vec4 w_pos = modelMatrix * vec4(in_position, 1.0f);
	vec4 w_pos =vec4(in_position, 1.0f);

	position = w_pos.xyz;
	gl_Position = projectionViewMatrix *  w_pos;
}
