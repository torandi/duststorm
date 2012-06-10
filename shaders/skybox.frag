#version 330
#include "uniforms.glsl"

#include "skybox_color.glsl"

in vec3 texcoord;
out vec4 ocolor;

void main() {
	ocolor.rgb = skybox_color(texcoord);
	ocolor.a=1.0;
}
