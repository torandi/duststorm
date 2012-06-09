vec3 skybox_color(vec3 texcoord) {
	vec4 tex_color = textureCube(texture_cube1, texcoord.xyz);
	return tex_color.rgb;
}
