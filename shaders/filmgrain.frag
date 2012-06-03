#version 330
#include "uniforms.glsl"

in vec2 uv;
out vec4 ocolor;

const float offset_ratio = 0.6f;
const float noise_ratio = 0.3f;

float rand(vec2 co){
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233) * (state.time + 1.0f))) * 43758.5453);
}

void main() {
	/* offset */
	float x = uv.x * uv.y * 1337.0f * state.time;
	x = mod(x,13.0f) * mod(x,123.0f);
	float dx = mod(x, 0.003f) - 0.0015f;
	float dy = mod(x, 0.002f) - 0.001f;

	/* noise */
	float noise = 1.0f - clamp(rand(uv) * noise_ratio, 0.0, 0.7);

	/* color */
	vec3 t1 = texture2D(texture1, uv).rgb;
	vec3 t2 = texture2D(texture1, uv + vec2(dx,dy)).rgb;

	ocolor.rgb = mix(t1, t2, offset_ratio) * noise;
	ocolor.a = 1.0f;
}
