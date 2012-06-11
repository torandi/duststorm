#version 330
#include "uniforms.glsl"

uniform bool use_light_a;
uniform bool use_light_b;
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
in vec4 normal;
in vec3 tangent;
in vec3 bitangent;
in vec4 color;
out vec4 ocolor;

void main() {
	if ( show_texture        ) ocolor = texture(texture1, uv);
	else if ( show_uv        ) ocolor = vec4(uv.x, 1-uv.y, 0.0f, 1.0f);
	else if ( show_normal    ) ocolor = vec4(normal.xyz * 0.5 + 0.5, 1.0f);
	else if ( show_normalmap ) ocolor = texture(texture2, uv);
	else if ( show_tangent   ) ocolor = vec4(tangent.xyz * 0.5 + 0.5, 1.0f);
	else if ( show_bitangent ) ocolor = vec4(bitangent.xyz * 0.5 + 0.5, 1.0f);
	else if ( show_color     ) ocolor = color;
	else                       ocolor = vec4(0,0,0,1);
}
