const float LOG2 = 1.442695;
const vec3 fog_color = vec3(0.5843137254901961, 0.6980392156862745, 0.6980392156862745);

vec3 calculate_fog(in vec3 original_color) {
	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = exp2(-fog_density * fog_density * z * z * LOG2);
	fogFactor = 1.0 - clamp(fogFactor, 0.0, 1.0);
	return mix(original_color, fog_color, fogFactor);
}
