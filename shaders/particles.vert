#version 150
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

layout (location = 0) in vec4 position; //w is rotation
layout (location = 1) in vec4 color;
layout (location = 2) in float scale;
layout (location = 3) in int texture_index;

out ParticleData {
	vec4 color;
	float scale;
	float rotation;
	int texture_index;
} particleData;

void main() {
	vec4 pos = modelMatrix * vec4(position.xyz, 1.0);
	particleData.texture_index = texture_index;

	particleData.rotation = position.w;
	particleData.scale = scale;

	gl_Position = viewMatrix * pos;

	particleData.color = color;
}

