#version 150
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

layout (location = 0) in vec4 in_position; //w is used as scale
layout (location = 1) in vec4 in_color;

out ParticleData {
	vec4 color;
	float scale;
} particleData;

void main() {
	vec4 pos = modelMatrix * vec4(in_position.xyz, 1.0);
	particleData.scale = in_position.w;

	gl_Position = viewMatrix * pos;

	vec3 accumLighting = in_color.rgb * Lgt.ambient_intensity;

	for(int light = 0; int(light) < Lgt.num_lights; ++light) {
		vec3 light_distance = Lgt.lights[light].position.xyz - pos.xyz;

		vec3 lightIntensity;
		if(Lgt.lights[light].type == 0) {
			lightIntensity = Lgt.lights[light].intensity.rgb;	
		} else {
			float lightAttenuation = (1 / ( 1.0 + Lgt.lights[light].attenuation * length(light_distance)));
			lightIntensity =  lightAttenuation * Lgt.lights[light].intensity.rgb;
		}

		accumLighting += lightIntensity * in_color.rgb ;
	}

	particleData.color.rgb= clamp(accumLighting,0.0, 1.0);

	particleData.color.a = in_color.a;
}

