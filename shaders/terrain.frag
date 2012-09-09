#version 330
#include "uniforms.glsl"

#define TEXTURE_REPEAT 128.0

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;
in vec4 shadowmap_coord[maxNumberOfLights];

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

	vec3 pos_tangent_space;
	pos_tangent_space.x = dot(position, norm_tangent);
	pos_tangent_space.y = dot(position, norm_bitangent);
	pos_tangent_space.z = dot(position, norm_normal);

	vec4 color1, color2;
	float color_mix;
	vec2 texcoord_real = texcoord * TEXTURE_REPEAT;
	color1 = texture2DArray(texture_array0, vec3(texcoord_real, 0));
	color2 = texture2DArray(texture_array0, vec3(texcoord_real, 1));
	color_mix = texture(texture0, texcoord).g;
	vec4 originalColor = mix(color1, color2, color_mix);

	color1 = texture2DArray(texture_array1, vec3(texcoord_real, 0));
	color2 = texture2DArray(texture_array1, vec3(texcoord_real, 1));
	vec3 normal_map = mix(color1, color2, color_mix).xyz;

	normal_map = normalize(normal_map * 2.0 - 1.0);

	float shininess = 18.f;
	vec4 accumLighting = originalColor * vec4(Lgt.ambient_intensity,1.f);

	
	for(int light = 0; light < Lgt.num_lights; ++light) {
		if(in_light(Lgt.lights[light], position, shadowmap_coord[light])) {
			accumLighting += compute_lighting(
				Lgt.lights[light], originalColor, 
				pos_tangent_space, normal_map, camera_dir, 
				norm_normal, norm_tangent, norm_bitangent,
					Mtl.shininess, Mtl.specular);
		}
	}

	ocolor = calculate_fog(clamp(accumLighting,0.0, 1.0));
	ocolor.a = 1.f;
}
