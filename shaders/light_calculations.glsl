vec4 computeLighting(
	in light_data light, in vec4 originalColor, 
	in vec3 normal_map, in vec3 light_dir, 
	in vec3 camera_dir, in vec3 light_distance,
	float shininess, vec4 specular, float specular_intensity,
	bool use_diffuse, bool use_specular
	) {
	vec3 lightIntensity;

	//Light attenuation on if type == 1 (point light)
	if(light.type == 0) {
		lightIntensity = light.intensity.rgb;	
	} else { 
		float lightAttenuation = (1 / ( 1.0 + light.attenuation * length(light_distance)));
		lightIntensity =  lightAttenuation * light.intensity.rgb;
	}

	float LambertTerm = max( dot(light_dir, normal_map), 0.0);
	float specular_amount = 0.0;

	if( LambertTerm > 0.0) {
		//Apply specular
		specular_amount = pow(clamp(dot(reflect(-light_dir, normal_map), camera_dir), 0.0, 1.0), shininess);
	}

	vec3 diffuse = originalColor.rgb * LambertTerm * lightIntensity;	
	vec3 specular_color = specular.rgb * specular_amount * specular_intensity * length(diffuse);

	vec4 color = vec4(0.0);
	if(use_diffuse)
		color.rgb += diffuse;
	if(use_specular)
		color.rgb += specular_color;
	color.a = 1.0;

	return color;
/*
	float cosAngIncidence = dot(surfaceNormal, light_dir);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);

	vec3 viewDirection = normalize(camera_pos-world_pos);
	vec3 halfAngle = normalize(light_dir + viewDirection);
	float angleNormalHalf = acos(dot(halfAngle, surfaceNormal));
	float exponent = angleNormalHalf / Mtl.shininess;
	exponent = -(exponent * exponent);
	float gaussianTerm = exp(exponent);

	gaussianTerm = cosAngIncidence != 0.0 ? gaussianTerm : 0.0;
	return (originalColor*lightIntensity*cosAngIncidence) + (Mtl.specular * lightIntensity * gaussianTerm);
*/	
}
