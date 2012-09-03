#version 330
#include "uniforms.glsl"

#define TEXTURE_REPEAT 32.0

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;

#include "light_calculations.glsl"
#include "fog.glsl"

out vec4 ocolor;

void main() {
	vec3 norm_normal, norm_tangent, norm_bitangent;
	norm_normal = normalize(normal);
	norm_tangent = normalize(tangent);
	norm_bitangent = normalize(bitangent);

	vec3 camera_direction = normalize(camera_pos - position);

	//Convert to tangent space:
	vec3 camera_dir;
	camera_dir.x = dot(camera_direction, norm_tangent);
	camera_dir.y = dot(camera_direction, norm_bitangent);
	camera_dir.z = dot(camera_direction, norm_normal);

	vec4 color1, color2;
	float color_mix;
	vec2 texcoord_real = texcoord * TEXTURE_REPEAT;
	color1 = texture2DArray(texture_array0, vec3(texcoord_real, 0));
	color2 = texture2DArray(texture_array0, vec3(texcoord_real, 1));
	color_mix = texture(texture0, texcoord).r;
	vec4 originalColor = mix(color1, color2, color_mix);

	color1 = texture2DArray(texture_array1, vec3(texcoord_real, 0));
	color2 = texture2DArray(texture_array1, vec3(texcoord_real, 1));
	vec3 normal_map = mix(color1, color2, color_mix).xyz;

	normal_map = normalize(normal_map * 2.0 - 1.0);

	float shininess = 32.f;
	vec4 accumLighting = originalColor * vec4(Lgt.ambient_intensity,1.f);

	for(int light = 0; light < Lgt.num_lights; ++light) {
		vec3 light_distance = Lgt.lights[light].position.xyz - position;
		vec3 dir = normalize(light_distance);
		vec3 light_dir;
		

		//Convert to tangent space
		light_dir.x = dot(dir, norm_tangent);
		light_dir.y = dot(dir, norm_bitangent);
		light_dir.z = dot(dir, norm_normal);

		accumLighting += computeLighting(
				Lgt.lights[light], originalColor,
				normal_map, light_dir,
				camera_dir, length(light_distance),
				shininess, vec4(.03f),
				true, true);
	}

	ocolor.rgb = calculate_fog(clamp(accumLighting,0.0, 1.0).rgb);
	ocolor.a= 1.f;
}
