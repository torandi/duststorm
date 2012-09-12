#version 150

//NORMAL SHADER

#include "uniforms.glsl"

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

	vec4 originalColor;
	originalColor = texture(texture0, texcoord);
	originalColor*=Mtl.diffuse;

	//Normal map
	vec3 normal_map = normalize(texture(texture1, texcoord).xyz * 2.0 - 1.0);

	float shininess = Mtl.shininess * normalize(texture(texture2, texcoord)).length();

	vec4 accumLighting = originalColor * vec4(Lgt.ambient_intensity, 1.0);

	for(int light = 0; int(light) < Lgt.num_lights; ++light) {
		accumLighting += compute_lighting(
			Lgt.lights[light], originalColor, 
			pos_tangent_space, normal_map, camera_dir, 
			norm_normal, norm_tangent, norm_bitangent,
				shininess, Mtl.specular) *
		shadow_coefficient(Lgt.lights[light], position, shadowmap_coord[light]);
	}

	ocolor = calculate_fog(clamp(accumLighting,0.0, 1.0));

	ocolor.a *= texture(texture3, texcoord).r;
}
