vec3 skybox_color(vec3 texcoord) {
	vec4 tex_color = texture(texture_cube0, texcoord.xyz);
	return tex_color.rgb;
}
