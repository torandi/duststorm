#version 330

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;

in vec2 uv;
out vec4 ocolor;

void main(){
	vec4 t1 = texture(texture1, uv);
	vec4 t2 = texture(texture2, uv);
	vec4 t3 = texture(texture3, uv);
	vec4 t4 = texture(texture4, uv);
	vec4 t5 = texture(texture5, uv);

	ocolor = vec4(0,0,0,0);
	ocolor += t1 * t5.r;
	ocolor += t2 * t5.g;
	ocolor += t3 * t5.b;
	ocolor += t4 * t5.a;
}
