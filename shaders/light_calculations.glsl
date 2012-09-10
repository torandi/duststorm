float attenuation(in light_data light, float d){
	return 1.0 / ( light.constant_attenuation + light.linear_attenuation * d + light.quadratic_attenuation * d * d);
}

vec4 compute_point_light(
	in vec3 light_pos, in light_data light, in vec4 originalColor,
	in vec3 position, in vec3 normal, in vec3 camera_dir,
	float shininess, vec4 specular_color) {

	vec3 light_dir = position - light_pos;
	float light_distance = length(light_dir);
	light_dir = normalize(light_dir);

	float LambertTerm = max( dot(-light_dir, normal), 0.0);

	float specular_amount = 0.0;
	if( LambertTerm > 0.0) {
		specular_amount = pow(clamp(dot(reflect(light_dir, normal), camera_dir), 0.0, 1.0), shininess);
	}

	vec4 color = vec4(0.f);
	color.rgb += light.intensity.rgb * originalColor.rgb * LambertTerm;
	color.rgb += specular_color.rgb * specular_amount;
	color.a = 1.0;

	float attn = attenuation(light, light_distance);

	return color * attn;
}

vec4 compute_directional_light(
	in vec3 light_dir, in light_data light, in vec4 originalColor,
	in vec3 position, in vec3 normal, in vec3 camera_dir,
	float shininess, vec4 specular_color) {

	light_dir = normalize(light_dir);

	float LambertTerm = max( dot(-light_dir, normal), 0.0);

	float specular_amount = 0.0;
	if( LambertTerm > 0.0) {
		specular_amount = pow(clamp(dot(reflect(light_dir, normal), camera_dir), 0.0, 1.0), shininess);
	}

	vec3 diffuse  = light.intensity.rgb * originalColor.rgb * LambertTerm;
	vec3 specular = specular_color.rgb * specular_amount;

	vec4 color = vec4(0.0);
	color.rgb += diffuse;
	color.rgb += specular;
	color.a = 1.0;

	return color;;
}

vec4 compute_lighting(
	in light_data light, in vec4 originalColor,
	in vec3 position, in vec3 normal, in vec3 camera_dir,
	in vec3 norm_normal, in vec3 norm_tangent, in vec3 norm_bitangent,
	float shininess, vec4 specular_color) {
	
	//Convert light to tangent space:
	vec3 light_pos;
	light_pos.x = dot(light.position.xyz, norm_tangent);
	light_pos.y = dot(light.position.xyz, norm_bitangent);
	light_pos.z = dot(light.position.xyz, norm_normal);

	return light.is_directional * compute_directional_light(light_pos, light, originalColor, position, normal, camera_dir, shininess, specular_color) 
	+ (1 - light.is_directional) * compute_point_light(light_pos, light, originalColor, position, normal, camera_dir, shininess, specular_color);
}

float offset_lookup(in light_data light, in sampler2D map, in vec3 loc, in vec2 offset) {
	return texture(map, loc.xy + offset * light.shadowmap_scale).r + light.shadow_bias >=  loc.z ? 1.f : 0.f;
}

float shadowmap_coef(in light_data light, in sampler2D shadowmap, in vec3 tex_coords) {
	float bias = light.shadow_bias;
	float sum = 0;
	float x, y;

	for (y = -1.5; y <= 1.5; y += 1.0) {
	  for (x = -1.5; x <= 1.5; x += 1.0) {
			sum += offset_lookup(light, shadowmap, tex_coords, vec2(x, y));
		}
	}
	return sum/16.f;
}

float shadow_coefficient(in light_data light, in vec3 position, in vec4 shadowmap_coord) {
	vec3 tex_coords = shadowmap_coord.xyz / shadowmap_coord.w;
	if( tex_coords.x > 0.f && tex_coords.x < 1.f
		&& tex_coords.y > 0.f && tex_coords.y < 1.f) {

		float coef;
		switch(light.shadowmap_index) {
			case 0:
				coef = shadowmap_coef(light, shadowmap0, tex_coords);
				break;
			case 1:
				coef = shadowmap_coef(light, shadowmap1, tex_coords);
				break;
			case 2:
				coef = shadowmap_coef(light, shadowmap2, tex_coords);
				break;
			case 3:
				coef = shadowmap_coef(light, shadowmap3, tex_coords);
				break;
		}

		return coef;
	} else {
		return light.is_directional; //Directional lights should display areas outside of the shadowmap as lit
	}
}
