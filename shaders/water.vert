#version 150
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

uniform float time;
uniform vec2 wave1;
uniform vec2 wave2;

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec4 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_bitangent;

out vec3 position;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;
out vec2 tex_coord1;
out vec2 tex_coord2;

void main() {
	vec4 pos = in_position;
	//pos.y = water_height;
	vec4 w_pos = modelMatrix * pos;
	position = w_pos.xyz;
	gl_Position = projectionViewMatrix *  w_pos;
//	depth = abs(water_height - in_position.y);

	normal = (normalMatrix * in_normal).xyz;
	tangent = (normalMatrix * in_tangent).xyz;
	bitangent = (normalMatrix * in_bitangent).xyz;

	tex_coord1 = in_texcoord + time*wave1;
	tex_coord2 = in_texcoord + time*wave2;

}

