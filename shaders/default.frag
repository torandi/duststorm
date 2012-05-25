#version 330

uniform sampler2D texture1;
in vec2 uv;
out vec4 ocolor;

void main(){
	ocolor = texture2D(texture1, uv);
}
