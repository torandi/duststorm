#version 330
#include "uniforms.glsl"

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in ParticleData {
	vec4 color;
	float scale;
	float rotation;
	int texture_index;
} particleData[];

out vec4 color;
out vec2 tex_coord;
flat out int texture_index;

void main() {
	float s = particleData[0].scale*0.1;
	float a = s * cos(particleData[0].rotation);
	float b = s * sin(particleData[0].rotation);
	color = particleData[0].color;
	texture_index = particleData[0].texture_index;


	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(a+b, -a+b, 0, 0.0));
	tex_coord = vec2(1,1);
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(a-b, a+b, 0, 0.0));
	tex_coord = vec2(1,0);
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-a+b, -a-b, 0, 0.0));
	tex_coord = vec2(0,1);
	EmitVertex();

	gl_Position = projectionMatrix * (gl_in[0].gl_Position + vec4(-a-b, a-b, 0, 0.0));
	tex_coord = vec2(0,0);
	EmitVertex();



	EndPrimitive();
}

