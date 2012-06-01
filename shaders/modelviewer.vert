#version 150
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texcoord;
out vec2 texcoord;

void main() {
   vec4 w_pos = modelMatrix * in_position;
   gl_Position = projectionViewMatrix *  w_pos;
   texcoord = in_texcoord;
}
