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
	vec4 v4_diff_color_2 = vec4(c,0.1,0.1, c * 2.);
	float z = gl_FragCoord.z / gl_FragCoord.w;
	z = 1. - (z * z) * 0.00002;
	v4_diff_color_2.rgb *= z;
	gl_FragColor = v4_diff_color_2;
	// const float LOG2 = 1.442695;
	// float z = gl_FragCoord.z / gl_FragCoord.w;
	// const float fog_density = 0.005;
	// float fog_factor = exp2( -fog_density * fog_density * z * z * LOG2 );
	// fog_factor = clamp(fog_factor, 0.0, 1.0);

	// gl_FragColor = mix(vec4(0.), vec4(v4_diff_color_2.rgb * 1.5, v4_diff_color_2.a), fog_factor);
}
