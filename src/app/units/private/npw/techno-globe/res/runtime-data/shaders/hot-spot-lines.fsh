#ifdef GL_ES
	precision lowp float;
#endif

uniform sampler2D u_s2d_tex;
uniform vec4 u_vertex_color;

varying vec2 v_v2_tex_coord;

void main()
{
	vec4 v4_diff_color = texture2D(u_s2d_tex, v_v2_tex_coord);
	//v4_diff_color.a -= v4_diff_color.a / 6.0;
	//v4_diff_color.a = 2.;
	float z = gl_FragCoord.z / gl_FragCoord.w;
	z = 1. - (z * z) * 0.00002;
	v4_diff_color.rgb *= z;
	gl_FragColor = v4_diff_color;// * u_vertex_color;
	// const float LOG2 = 1.442695;
	// float z = gl_FragCoord.z / gl_FragCoord.w;
	// const float fog_density = 0.004;
	// float fog_factor = exp2( -fog_density * fog_density * z * z * LOG2 );
	// fog_factor = clamp(fog_factor, 0.0, 1.0);

	// gl_FragColor = mix(vec4(0.), vec4(v4_diff_color.rgb * 1.5, v4_diff_color.a), fog_factor);
}
