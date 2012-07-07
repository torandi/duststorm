#version 150
#include "uniforms.glsl"

#define RENDER_TANGENT 0
#define RENDER_NORMAL 1
#define RENDER_BITANGENT 0

const vec4 wireframe_color = vec4(0,1.0f,0,1);

layout (triangles) in;
layout (line_strip, max_vertices = 42) out;

in VertexData {
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} vertexData[];

out vec4 color;

void main() {
	float extra_lines_length = 0.3;

	#if RENDER_TANGENT || RENDER_NORMAL || RENDER_BITANGENT
	//Generate normals:
	for(int i = 0; i < gl_in.length(); ++i) {
		#if RENDER_NORMAL
			//normal:
			color = vec4(1.f, 0.0f, 0.0f, 1.f);
			gl_Position = projectionViewMatrix * gl_in[i].gl_Position;
			EmitVertex();

			color = vec4(1.f, 0.0f, 0.0f, 1.f);
			gl_Position = projectionViewMatrix * (gl_in[i].gl_Position+ vec4(normalize(vertexData[i].normal)*extra_lines_length, 0.f));
			EmitVertex();
			EndPrimitive();
		#endif

		#if RENDER_TANGENT
			//tangent:
			color = vec4(0.f, 0.0f, 1.0f, 1.f);
			gl_Position = projectionViewMatrix * gl_in[i].gl_Position;
			EmitVertex();

			color = vec4(0.f, 0.0f, 1.0f, 1.f);
			gl_Position = projectionViewMatrix * (gl_in[i].gl_Position+ vec4(normalize(vertexData[i].tangent)*extra_lines_length, 0.f));
			EmitVertex();
			EndPrimitive();
		#endif

		#if RENDER_BITANGENT
			//bitangent:
			color = vec4(0.f, 1.0f, 0.0f, 1.f);
			gl_Position = projectionViewMatrix * gl_in[i].gl_Position;
			EmitVertex();

			color = vec4(0.f, 1.0f, 0.0f, 1.f);
			gl_Position = projectionViewMatrix * (gl_in[i].gl_Position+ vec4(normalize(vertexData[i].bitangent)*extra_lines_length, 0.f));
			EmitVertex();
			EndPrimitive();
		#endif
	}
	#endif

	//Outlines:
	color = wireframe_color;
	gl_Position = projectionViewMatrix * gl_in[0].gl_Position;
	EmitVertex();
	color = wireframe_color;
	gl_Position = projectionViewMatrix * gl_in[1].gl_Position;
	EmitVertex();
	EndPrimitive();

	gl_Position = projectionViewMatrix * gl_in[1].gl_Position;
	EmitVertex();
	color = wireframe_color;
	gl_Position = projectionViewMatrix * gl_in[2].gl_Position;
	EmitVertex();
	color = wireframe_color;
	EndPrimitive();

	gl_Position = projectionViewMatrix * gl_in[0].gl_Position;
	EmitVertex();
	color = wireframe_color;
	gl_Position = projectionViewMatrix * gl_in[2].gl_Position;
	EmitVertex();
	color = wireframe_color;
	EndPrimitive();
}
