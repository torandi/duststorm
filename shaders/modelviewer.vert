#version 410
#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;
layout (location = 5) in vec4 in_color;

out vec2 uv;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;
out vec4 color;

void main() {
   vec4 w_pos = modelMatrix * in_position;
   gl_Position = projectionViewMatrix *  w_pos;

   /* passthru */
   uv = in_uv;
   normal = in_normal;
   tangent = in_tangent;
   bitangent = in_bitangent;
   color = in_color;
}
