#version 150
#include "uniforms.glsl"
#include "fog.glsl"

in vec4 color;

out vec4 ocolor;

void main() {
	ocolor = calculate_fog(color);
}
