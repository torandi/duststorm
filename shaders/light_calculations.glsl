float attenuation(in light_data light, float d){
	return 1.0 / ( light.constant_attenuation + light.linear_attenuation * d + light.quadratic_attenuation * d * d);
}

vec4 computeLighting(
	in light_data light, in vec4 originalColor,
	in vec3 N, in vec3 L,
	in vec3 camera_dir, in float distance,
	float shininess, vec4 specular_color,
	bool use_diffuse, bool use_specular
	) {

	float LambertTerm = max( dot(L, N), 0.0);

	float specular_amount = 0.0;
	if( LambertTerm > 0.0) {
		specular_amount = pow(clamp(dot(reflect(-L, N), camera_dir), 0.0, 1.0), shininess);
	}

	vec3 diffuse  = light.intensity.rgb * originalColor.rgb * LambertTerm;
	vec3 specular = specular_color.rgb * specular_amount;

	vec4 color = vec4(0.0);
	if(use_diffuse)
		color.rgb += diffuse;
	if(use_specular)
		color.rgb += specular;
	color.a = 1.0;

	float attn = 1.0;
//	if ( light.type == 1 ){ /* point light */
		attn = attenuation(light, distance);
//	}

	return color * attn;
}
