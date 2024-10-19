shader_type canvas_item;

/** Value between 0 and 1 describing how far along the transition animation we are. */
uniform float progress : hint_range(0.0, 1.0) = 0.0;
/** The colour of the transition. */
uniform vec3 color : source_color = vec3(0., 0., 0.);

void fragment() {
	COLOR = texture(TEXTURE, UV);
	COLOR.a = clamp(COLOR.r - (progress * 2. - 1.), 0., 1.);
	COLOR.rgb = color;
}
