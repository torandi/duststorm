#version 330
#include "uniforms.glsl"

in vec2 uv;
out vec4 ocolor;

void main(){
	float s = max(sin(state.time*0.7f), 0.0f);
	float m = abs(sin(state.time*2)) * s * 0.4;
	vec2 offset = vec2(uv.x + sin(uv.y * 7 + state.time * 2) * 0.05 * s, uv.y);
	vec4 t1 = texture2D(texture1, offset);
	vec4 t2 = texture2D(texture2, offset) * 1.2f;

	ocolor = mix(t1, t2, s) + vec4(1,1,1,1)*m;
}
