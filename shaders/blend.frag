#version 330
#include "uniforms.glsl"

in vec2 uv;
out vec4 ocolor;

void main(){
	vec4 s = texture(texture4, uv);

	ocolor = vec4(0,0,0,0);
	ocolor += texture(texture0, uv) * s.r;
	ocolor += texture(texture1, uv) * s.g;
	ocolor += texture(texture2, uv) * s.b;
	ocolor += texture(texture3, uv) * s.a;
}
