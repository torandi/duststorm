#version 150
#include "uniforms.glsl"

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;

out vec4 color;

void main() {
	vec4 pos = modelMatrix * in_position;

   gl_Position = viewMatrix * pos;

   vec4 accumLighting = in_color * vec4(Lgt.ambient_intensity, 1.0);

   for(int light = 0; uint(light) < Lgt.num_lights; ++light) {
      vec3 light_distance = Lgt.lights[light].position.xyz - pos.xyz;

      vec3 lightIntensity;
      if(Lgt.lights[light].type == 0) {
         lightIntensity = Lgt.lights[light].intensity.rgb;	
      } else { 
         float lightAttenuation = (1 / ( 1.0 + Lgt.lights[light].attenuation * length(light_distance)));
         lightIntensity =  lightAttenuation * Lgt.lights[light].intensity.rgb;
      }

      accumLighting += vec4(lightIntensity, 1.0)*in_color;
   }


   color= clamp(accumLighting,0.0, 1.0);
}

