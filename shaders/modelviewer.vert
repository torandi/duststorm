#version 410
#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_bitangent;
layout (location = 5) in vec4 in_color;

/* unmodified values */
out vec2 uv;
out vec4 color;
out vec3 orig_normal;
out vec3 orig_tangent;
out vec3 orig_bitangent;

/* multiplied values */
out vec3 mul_position;
out vec3 mul_normal;
out vec3 mul_tangent;
out vec3 mul_bitangent;

void main() {
   vec4 mul_pos = modelMatrix * in_position;
   gl_Position = projectionViewMatrix *  mul_pos;

   /* passthru */
   uv             = in_uv;
   color          = in_color;
   orig_normal    = in_normal.xyz;
   orig_tangent   = in_tangent.xyz;
   orig_bitangent = in_bitangent.xyz;

   /* multiplied */
   mul_normal     = (normalMatrix * in_normal).xyz;
   mul_tangent    = (normalMatrix * in_tangent).xyz;
   mul_bitangent  = (normalMatrix * in_bitangent).xyz;
}
