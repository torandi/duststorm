#version 150
#include "uniforms.glsl"

in vec2 texcoord;
in vec4 color;

out vec4 ocolor;

void main() {
	//vec4 tex_color = texture(texture1, texcoord);
	//ocolor = tex_color*color;
   ocolor = vec4(0, 1, 0, 1);

}
