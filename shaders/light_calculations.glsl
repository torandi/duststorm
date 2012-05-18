vec4 computeLighting(in vec4 originalColor,
	in vec3 normal_map, in vec3 light_dir, 
	in vec3 camera_dir, in vec3 light_distance
	) {
	vec3 lightIntensity;


	float lightAttenuation = (1 / ( 1.0 + light.attenuation * length(light_distance)));
	lightIntensity =  lightAttenuation * light.intensity.rgb;

	float LambertTerm = max( dot(light_dir, normal_map), 0.0);
	float specular_amount = 0.0;

	if( LambertTerm > 0.0) {
		//Apply specular
		specular_amount = pow(clamp(dot(reflect(-light_dir, normal_map), camera_dir), 0.0, 1.0), shininess);
	}

	vec3 diffuse = originalColor.rgb * LambertTerm * lightIntensity;	
	vec3 specular_color = specular.rgb * specular_amount * specular_intensity * length(diffuse);

	vec4 color = vec4(0.0);
/*
	if(use_diffuse)
		color.rgb += diffuse;
	if(use_specular)
		color.rgb += specular_color;
		*/
	color.a = 1.0;

	return color;
}
