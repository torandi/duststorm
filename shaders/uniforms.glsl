#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_attrib_location: enable

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform sampler2D texture6;
uniform sampler2D texture7;
uniform sampler2DArray texture_array0;
uniform sampler2DArray texture_array1;
uniform sampler2DArray texture_array2;
uniform sampler2DArray texture_array3;
uniform samplerCube texture_cube0;
uniform samplerCube texture_cube1;
uniform samplerCube texture_cube2;
uniform samplerCube texture_cube3;

const int maxNumberOfLights = 4;

layout(std140) uniform projectionViewMatrices {
   mat4 projectionViewMatrix;

   mat4 projectionMatrix;
   mat4 viewMatrix;
};

layout(std140) uniform modelMatrices {
   mat4 modelMatrix;
   mat4 normalMatrix;
};

layout(std140) uniform Camera {
	vec3 camera_pos; //The cameras position in world space
};


layout(std140) uniform Material {
	float shininess;
	vec4 diffuse;
	vec4 specular;
	vec4 ambient;
	vec4 emission;
} Mtl;

struct light_data {
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	int type;
	vec3 intensity;
	vec3 position;
};

layout(std140) uniform LightsData {
	int num_lights;
	vec3 ambient_intensity;
	light_data lights[maxNumberOfLights];
} Lgt;

layout(std140) uniform StateData {
	float time;
	float width;
	float height;
} state;
