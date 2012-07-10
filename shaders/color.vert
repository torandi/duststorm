#version 330
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

layout (location=0) in vec4 in_pos;

void main(){
	vec4 w_pos = modelMatrix * in_pos;
	gl_Position = projectionViewMatrix *  w_pos;
}
