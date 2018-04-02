#ifdef GL_ES
	precision lowp float;
#endif

void main()
{
	vec3 v3_diff_color = vec3(1., 0., 0.);
	
	float z = gl_FragCoord.z / gl_FragCoord.w;
	z = 1. - (z * z) * 0.00002;
	gl_FragColor = vec4(v3_diff_color * z, 1.);
	// const float LOG2 = 1.442695;
	// float z = gl_FragCoord.z / gl_FragCoord.w;
	// const float fog_density = 0.005;
	// float fog_factor = exp2( -fog_density * fog_density * z * z * LOG2 );
	// fog_factor = clamp(fog_factor, 0.0, 1.0);

	// gl_FragColor = mix(vec4(0.), vec4(v3_diff_color * 1.5, 1.), fog_factor);
}
