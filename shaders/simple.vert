#version 150

#include "uniforms.glsl"

layout(location=0) in vec3 in_position;
layout(location=1) in vec4 in_color;

out vec4 color;

void main() {
	vec4 w_pos = modelMatrix * vec4(in_position, 1.0f);

	color = in_color;

	gl_Position = projectionViewMatrix * w_pos;
}
