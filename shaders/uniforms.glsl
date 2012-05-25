
uniform sampler2D texture1;
uniform sampler2D texture2;

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
	float attenuation;
   int type;
	vec4 intensity;
	vec4 position;
};

layout(std140) uniform LightsData {
	int num_lights;
	vec3 ambient_intensity;
	light_data lights[maxNumberOfLights];
} Lgt;

