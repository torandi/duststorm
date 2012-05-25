#version 330
#extension GL_ARB_explicit_attrib_location: enable

uniform sampler2D texture1;
in vec2 uv;
out vec4 ocolor;

void main(){
	ocolor = texture2D(texture1, uv);
}
