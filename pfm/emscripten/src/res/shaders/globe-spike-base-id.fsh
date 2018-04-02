#ifdef GL_ES
	precision lowp float;
#endif

varying float v_v1_vx_id;

void main()
{
	gl_FragColor.r = v_v1_vx_id;
	gl_FragColor.gb = vec2(0.);
	gl_FragColor.a = 1.;
}
