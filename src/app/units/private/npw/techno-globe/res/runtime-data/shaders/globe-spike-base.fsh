#ifdef GL_ES
	precision lowp float;
#endif

varying vec2 v_v2_tex_coord;

void main()
{
	vec2 v2_coord = abs(v_v2_tex_coord - vec2(0.5)) * 2.;
	float dist = length(v2_coord);
	float c = 1. - dist;
	vec3 v3_diff_color = vec3(1., 0., 0.);
	
	gl_FragColor = vec4(c,0.1,0.1, c * 2.);
}
