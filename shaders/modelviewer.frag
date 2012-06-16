#version 330
#include "uniforms.glsl"
#include "light_calculations.glsl"

uniform bool use_light[2];
uniform bool show_shaded;
uniform bool show_texture;
uniform bool show_uv;
uniform bool show_normal;
uniform bool show_normalmap;
uniform bool show_tangent;
uniform bool show_bitangent;
uniform bool show_color;
uniform bool show_diffuse;
uniform bool show_specular;

in vec2 uv;
in vec4 color;

in vec4 orig_normal;
in vec3 orig_tangent;
in vec3 orig_bitangent;
in vec3 mul_position;
in vec3 mul_normal;
in vec3 mul_tangent;
in vec3 mul_bitangent;

out vec4 ocolor;

void main() {
	vec4 t1 = texture(texture1, uv);
	vec4 t2 = texture(texture2, uv);

	if ( show_texture        ) ocolor = t1;
	else if ( show_uv        ) ocolor = vec4(uv.x, 1-uv.y, 0.0f, 1.0f);
	else if ( show_normal    ) ocolor = vec4(orig_normal.xyz * 0.5 + 0.5, 1.0f);
	else if ( show_normalmap ) ocolor = t2;
	else if ( show_tangent   ) ocolor = vec4(orig_tangent.xyz * 0.5 + 0.5, 1.0f);
	else if ( show_bitangent ) ocolor = vec4(orig_bitangent.xyz * 0.5 + 0.5, 1.0f);
	else if ( show_color     ) ocolor = color;
	else if ( show_shaded ){
		vec3 tmp = normalize(camera_pos - mul_position);
		vec3 camera_dir = vec3(
			dot(tmp, normalize(mul_tangent)),
			dot(tmp, normalize(mul_bitangent)),
			dot(tmp, normalize(mul_normal)));

		vec3 normal_map = normalize(t2.xyz * 2.0 - 1.0);

		vec4 lit = vec4(0,0,0,1);
		ocolor = t1;

		for ( int i = 0; i < 2; i++ ){
			if ( use_light[i] ){
				vec3 light_distance = Lgt.lights[i].position.xyz - mul_position;
				vec3 dir = normalize(light_distance);
				vec3 light_dir = vec3(
					dot(dir, normalize(mul_tangent)),
					dot(dir, normalize(mul_bitangent)),
					dot(dir, normalize(mul_normal)));

				lit += computeLighting(
					Lgt.lights[i], ocolor, normal_map,
					light_dir, camera_dir, length(light_distance),
					Mtl.shininess, Mtl.specular,
					show_diffuse, show_specular);
			}
		}

		ocolor = vec4(lit.rgb, 1);
	} else {
		ocolor = vec4(0,0,0,1);
	}

	ocolor= clamp(ocolor, 0.0, 1.0);
}
