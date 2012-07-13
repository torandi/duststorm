#version 330
#include "uniforms.glsl"

uniform float fog_density;
uniform vec2 marker_position;
uniform float marker_size;

#define TEXTURE_REPEAT 16.0


in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;

#include "light_calculations.glsl"

out vec4 ocolor;

const float LOG2 = 1.442695;
const vec3 fog_color = vec3(0.5843137254901961, 0.6980392156862745, 0.6980392156862745);

void main() {
	vec3 norm_normal, norm_tangent, norm_bitangent;
	norm_normal = normalize(normal);
	norm_tangent = normalize(tangent);
	norm_bitangent = normalize(bitangent);

	vec3 camera_direction = normalize(camera_pos - position);

	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = exp2(-fog_density * fog_density * z * z * LOG2);
	fogFactor = 1.0 - clamp(fogFactor, 0.0, 1.0);

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

	vec4 extra_light = vec4(0, 0, 0, 0); //added from marker

	vec2 mdiff = texcoord - marker_position;
	if(mdiff.x < marker_size && mdiff.x > 0 && mdiff.y > 0 && mdiff.y < marker_size) {
		vec4 marker_color = texture(texture1, mdiff/marker_size);
		originalColor.rgb = mix(originalColor.rgb,marker_color.rgb,  marker_color.a);
		extra_light = vec4(0.8)*marker_color.a;
	}

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
	accumLighting += originalColor * extra_light;

	ocolor.rgb = mix(clamp(accumLighting,0.0, 1.0).rgb, fog_color, fogFactor);
	ocolor.a= 1.f;
}
