#version 150
#extension GL_EXT_gpu_shader4 : enable
#include "uniforms.glsl"
#include "screenspace.glsl"

in vec4 color;
in vec2 tex_coord;
flat in int texture_index;

const float fade_scale = 7.0;

out vec4 ocolor;

void main() {
	float screen_depth = linear_depth(texture0, camera_near, camera_far);

	float particle_depth = (2.0 * camera_near) / (camera_far + camera_near - (gl_FragCoord.z/gl_FragCoord.w) * (camera_far - camera_near));
	float fade = clamp( (screen_depth - particle_depth) * fade_scale, 0.0, 1.0) ;

	vec4 tex_color = texture2DArray(texture_array0, vec3(tex_coord, texture_index));
	ocolor = tex_color*color;
	ocolor.a *= fade;
}
