/**
 * Calculate screenspace UV based on texture unit 0.
 * This is intended for shaders dealing with post-effects only.
 */
vec2 screenspace_uv(){
	ivec2 size = textureSize(texture0, 0);
	return vec2(gl_FragCoord.x / size.x, gl_FragCoord.y / size.y);
}
