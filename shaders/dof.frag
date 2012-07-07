#version 330
#include "uniforms.glsl"
#include "screenspace.glsl"

in vec2 uv;
out vec4 ocolor;

void main(){
	//ocolor.rgb = linear_depth(texture1, 0.1f, 100.0f) * vec3(1,1,1);
	ocolor.rgb = texture(texture0, uv).rgb;
	ocolor.a = 1.0f;
}
