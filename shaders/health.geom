#version 330
#include "uniforms.glsl"

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in BarData {
	vec4 color;
	float scale;
} barData[];

out vec4 color;

void main() {
	float s = 0.3 * barData[0].scale;
	float y = 0.1;

	color = barData[0].color;


	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(s, -y, 0, 0.0));
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(s, y, 0, 0.0));
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-s, -y, 0, 0.0));
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-s, y, 0, 0.0));
	EmitVertex();



	EndPrimitive();
}

