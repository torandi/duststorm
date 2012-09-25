#version 150
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

layout (location = 0) in float life;
layout (location = 1) in float scale;

out BarData {
	vec4 color;
	float scale;
} barData;

void main() {
	vec4 pos = modelMatrix * vec4(0, 1.0, 0, 1);

	barData.scale = scale;

	gl_Position = viewMatrix * pos;

	barData.color = mix(vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), life);
}

