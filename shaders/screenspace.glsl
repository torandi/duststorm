/**
 * Calculate screenspace UV based on texture.
 */
vec2 screenspace_uv(sampler2D t){
	ivec2 size = textureSize(t, 0);
	return vec2(gl_FragCoord.x / size.x, gl_FragCoord.y / size.y);
}
