#version 150
#include "uniforms.glsl"

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 in_color[];
in float scale[];

out vec4 color;
out vec2 tex_coord;

void main() {
	float d = scale[0]*0.5;
	color = in_color[0];

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(d, d, 0, 1.0));
	tex_coord = vec2(1,0);
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(d, -d, 0, 1.0));
	tex_coord = vec2(1,1);
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-d, d, 0, 1.0));
	tex_coord = vec2(0,0);
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-d, -d, 0, 1.0));
	tex_coord = vec2(0,1);
	EmitVertex();


	EndPrimitive();

}

