#version 330
#include "uniforms.glsl"
#include "screenspace.glsl"

in vec2 uv;
out vec4 ocolor;

void main(){
	float depth = linear_depth(texture1, 0.1f, 150.0f);

	vec3 t0 = texture(texture0, uv).rgb;
	vec3 t1 = texture(texture2, uv).rgb;

	ocolor.rgb = mix(t0, t1, depth);
	ocolor.a = 1.0f;
}
