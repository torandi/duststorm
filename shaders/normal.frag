#version 150

//NORMAL SHADER

#include "uniforms.glsl"

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;

#include "light_calculations.glsl"

out vec4 ocolor;

void main() {

	vec3 norm_normal, norm_tangent, norm_bitangent;
	norm_normal = normalize(normal);
	norm_tangent = normalize(tangent);
	norm_bitangent = normalize(bitangent);

	vec3 camera_direction = normalize(camera_pos - position);

	//Convert to tangent space:
	vec3 camera_dir;
	camera_dir.x = dot(camera_direction, norm_tangent);
	camera_dir.y = dot(camera_direction, norm_bitangent);
	camera_dir.z = dot(camera_direction, norm_normal);

	vec4 originalColor;
   originalColor = texture(texture0, texcoord);
   //originalColor*=Mtl.diffuse;

	vec3 normal_map = vec3(0.0, 0.0, 1.0);

	/* //Normal map
		normal_map = normalize(texture(texture2, texcoord).xyz * 2.0 - 1.0);
	*/

   vec4 accumLighting = originalColor * vec4(Lgt.ambient_intensity, 1.0);

   for(int light = 0; int(light) < Lgt.num_lights; ++light) {
      vec3 light_distance = Lgt.lights[light].position.xyz - position;
      vec3 dir = normalize(light_distance);
      vec3 light_dir;

      //Convert to tangent space
      light_dir.x = dot(dir, norm_tangent);
      light_dir.y = dot(dir, norm_bitangent);
      light_dir.z = dot(dir, norm_normal);

      accumLighting += computeLighting(
            Lgt.lights[light], originalColor, normal_map,
            light_dir, camera_dir, length(light_distance),
            Mtl.shininess, Mtl.specular,
            true, true);
   }


   ocolor= clamp(accumLighting,0.0, 1.0);
}
