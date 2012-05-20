#version 330

#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texCoord;
layout (location = 2) in vec4 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_bitangent;

out VertexData {
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} vertexData;

void main() {
	gl_Position  = modelMatrix * in_position;
	vertexData.normal = (normalMatrix * in_normal).xyz;
	vertexData.tangent = (normalMatrix * in_tangent).xyz;
	vertexData.bitangent = (normalMatrix * in_bitangent).xyz;
}
