#ifdef GL_ES
	precision lowp float;
#endif

void main()
{
	vec3 v3_diff_color = vec3(1., 0., 0.);
	
	gl_FragColor = vec4(v3_diff_color, 1.);
}
