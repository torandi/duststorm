#version 150
#extension GL_ARB_explicit_attrib_location: enable

#include "uniforms.glsl"

layout (location = 0) in vec4 position; //w is rotation
layout (location = 1) in vec4 color;
layout (location = 2) in float scale;
layout (location = 3) in int texture_index;

out ParticleData {
	vec4 color;
	float scale;
	float rotation;
	int texture_index;
} particleData;

void main() {
	vec4 pos = modelMatrix * vec4(position.xyz, 1.0);
	particleData.texture_index = texture_index;

	particleData.rotation = position.w;

	particleData.scale = scale;
	gl_Position = viewMatrix * pos;

	vec3 accumLighting = color.rgb * Lgt.ambient_intensity;

	for(int light = 0; int(light) < Lgt.num_lights; ++light) {
		vec3 light_distance = Lgt.lights[light].position.xyz - pos.xyz;

		vec3 lightIntensity;
		if(Lgt.lights[light].type == 0) {
			lightIntensity = Lgt.lights[light].intensity.rgb;	
		} else {
			float lightAttenuation = (1 / ( 1.0 + Lgt.lights[light].attenuation * length(light_distance)));
			lightIntensity =  lightAttenuation * Lgt.lights[light].intensity.rgb;
		}

		accumLighting += lightIntensity * color.rgb ;
	}
	
	particleData.color.rgb= clamp(accumLighting,0.0, 1.0);

	particleData.color.a = color.a;
}

