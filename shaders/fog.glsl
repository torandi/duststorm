const float LOG2 = 1.442695;

vec4 calculate_fog(in vec4 original_color) {
	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = exp2(-fog_density * fog_density * z * z * LOG2);
	fogFactor = 1.0 - clamp(fogFactor, 0.0, 1.0);
	return mix(original_color, fog_color, fogFactor);
}
