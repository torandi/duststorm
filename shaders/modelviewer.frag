#version 150
#include "uniforms.glsl"

in vec2 texcoord;
out vec4 ocolor;

void main() {
	ocolor = texture(texture1, texcoord);
}
