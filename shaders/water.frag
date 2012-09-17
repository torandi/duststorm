#version 150
#include "uniforms.glsl"
#include "fog.glsl"

uniform vec3 water_tint;

//const float water_sight = 5.0;

//const vec3 water_tint = vec3(0.8, 1.0, 1.0);
const vec4 specular=vec4(1.0);
const float shininess = 64.0;

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 tex_coord1;
in vec2 tex_coord2;

#include "skybox_color.glsl"
#include "light_calculations.glsl"

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
	camera_dir = normalize(camera_dir);

	vec3 pos_tangent_space;
	pos_tangent_space.x = dot(position, norm_tangent);
	pos_tangent_space.y = dot(position, norm_bitangent);
	pos_tangent_space.z = dot(position, norm_normal);

	vec3 normal_map1 = texture(texture1, tex_coord1).xyz;
	vec3 normal_map2 = texture(texture1, tex_coord2).xyz;

	vec3 normal_map = normalize((normal_map1 + normal_map2).xyz * 2.0 - 1.0);

	vec3 r = reflect(-camera_dir, normal_map);
	vec4 originalColor;
	originalColor.rgb = skybox_color(r)*water_tint;
	//originalColor.rgb = water_tint;

	vec4 accumLighting = originalColor * vec4(Lgt.ambient_intensity,1.0) *1.5;

	for(int light = 0; int(light) < Lgt.num_lights; ++light) {
		accumLighting += compute_lighting(
			Lgt.lights[light], originalColor, 
			pos_tangent_space, normal_map, camera_dir, 
			norm_normal, norm_tangent, norm_bitangent,
			shininess, Mtl.specular);
	}
	vec3 surface = clamp(accumLighting,0.0, 1.0).rgb;

	//angle of vision
	//float aov = 1.0-abs(dot(camera_direction, norm_normal));

	/*float current_depth = (2.0 * camera_near) / (camera_far + camera_near - (gl_FragCoord.z/gl_FragCoord.w) * (camera_far - camera_near));
	float screen_depth = linear_depth(texture2, camera_near, camera_far);*/

	ocolor.rgb = surface.rgb;
	ocolor.a = 1.f;

	ocolor = calculate_fog(ocolor);
}
